
/*
 * Copyright (C) 2012 JJ Whg
 *   <jjwhgbw@gmail.com>
 *
 * This file is part of bwelo.
 * 
 * bwelo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bwelo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with bwelo.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _XOPEN_SOURCE 500

#include "html.h"
#include "game.h"
#include "global.h"
#include "player.h"
#include "player_list.h"

#include <ftw.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <talloc.h>
#include <time.h>
#include <unistd.h>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

#ifndef JS_TABLE_SORT_URL
#define JS_TABLE_SORT_URL "http://www.frequency-decoder.com/demo/table-sort-revisited/js/tablesort.min.js"
#endif

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/
struct player_list_table_iter_args
{
    FILE *file;
    void *pctx;
};

struct generate_player_page_args
{
    void *pctx;
    const char *outdir;
};

struct player_page_table_args
{
    void *pctx;
    FILE *file;
    const char *player_key;
};

struct map_list_table_iter_args
{
    FILE *file;
    void *pctx;
};

struct generate_map_page_args
{
    void *pctx;
    const char *outdir;
};

struct map_page_table_args
{
    void *pctx;
    FILE *file;
    const char *map_key;
};

/***********************************************************************
 * Static Method Headers                                               *
 ***********************************************************************/
static int nftw_rm_rf(const char *path, const struct stat *sb,
                      int type, struct FTW *ftwbuf);

static int write_header(void *pctx, FILE * file, const char *page_name);
static int write_footer(void *pctx, FILE * file);

/* Creates a new table.  id is what ends up inside the ID field of the
 * <table> tag.  sort_index is the index to start sorting by.
 * reverse_sort only controls the starting index's reverse sort. */
static int start_table(void *pctx, FILE * file, const char *id,
                       int sort_index, bool reverse_sort, ...);
static int table_row(void *pctx, FILE * file, ...);
static int end_table(void *pctx, FILE * file);

static int generate_index_page(void *pctx, const char *filename);

static int generate_player_list(void *ctx, const char *filename);
static int player_list_table_iter(struct player *player, void *args_uc);

static int generate_player_page(struct player *player, void *args);
static int player_page_table(struct game *game, void *args);

static int generate_map_list(void *ctx, const char *filename);
static int map_list_table_iter(struct map *map, void *args_uc);

static int generate_map_page(struct map *map, void *args);
static int map_page_table(struct game *game, void *args);

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
int html_generate(void *parent_context, const char *outdir)
{
    void *ctx;
    const char *index_filename;
    const char *player_list_filename;
    const char *map_list_filename;
    struct generate_player_page_args gpp_args;
    struct generate_map_page_args gmp_args;

    ctx = talloc_new(parent_context);
    if (ctx == NULL)
        return 1;

    /* This is an "rm -rf outdir", it cleans up everything in the
     * output directory */
    nftw(outdir, &nftw_rm_rf, 16, FTW_DEPTH);
    mkdir(outdir, 0777);

    /* The index page is very simple. */
    index_filename = talloc_asprintf(ctx, "%s/index.html", outdir);
    generate_index_page(ctx, index_filename);

    /* Generates the list of all players */
    player_list_filename = talloc_asprintf(ctx, "%s/players.html", outdir);
    generate_player_list(ctx, player_list_filename);

    /* Generates a page for every player in the database */
    gpp_args.pctx = ctx;
    gpp_args.outdir = outdir;
    player_list_each(global_player_list, generate_player_page, &gpp_args);

    /* Generates the list of all maps */
    map_list_filename = talloc_asprintf(ctx, "%s/maps.html", outdir);
    generate_map_list(ctx, map_list_filename);

    /* Generates a page for every player in the database */
    gmp_args.pctx = ctx;
    gmp_args.outdir = outdir;
    map_list_each(global_map_list, generate_map_page, &gmp_args);

    TALLOC_FREE(ctx);
    return 0;
}

/***********************************************************************
 * Static Methods                                                      *
 ***********************************************************************/
int nftw_rm_rf(const char *path,
               const struct stat *sb __attribute__ ((unused)),
               int type __attribute__ ((unused)),
               struct FTW *ftwbuf __attribute__ ((unused)))
{
    /* FIXME: I should really check the type against FTW_F and FTW_D,
     *        but it's always set to 0 -- why? */
    unlink(path);
    rmdir(path);

    return 0;
}

int write_header(void *pctx __attribute__ ((unused)),
                 FILE * file, const char *page_name)
{
    fprintf(file, "<html>\n");
    fprintf(file, "<head>\n");
    fprintf(file, "<title>%s</title>\n", page_name);

    /* Load the table sorting code */
    fprintf(file, "<script type=\"text/javascript\" src=\"%s\"></script>\n",
            JS_TABLE_SORT_URL);

    /* Alternate table backgrounds for readability */
    fprintf(file,
            "<style type=\"text/css\">"
            "tr:nth-child(even) {background: #EEE}" "</style>\n");
    fprintf(file,
            "<style type=\"text/css\">"
            "tr:nth-child(odd) {background: #FFF}" "</style>\n");

    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");

    return 0;
}

int write_footer(void *pctx __attribute__ ((unused)), FILE * file)
{
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");

    return 0;
}

int start_table(void *pctx __attribute__ ((unused)),
                FILE * file, const char *table_id, int sort_index,
                bool reverse_sort, ...)
{
    va_list args;
    const char *col_name;
    const char *reverse;

    va_start(args, reverse_sort);

    if (reverse_sort)
        reverse = "r";
    else
        reverse = "";

    fprintf(file, "<table id=\"%s\""
            "class=\"sortable-onload-%d%s\""
            "class=\"rowstyle-alternate\""
            ">\n", table_id, sort_index, reverse);

    fprintf(file, "<thead><tr>\n");
    while ((col_name = va_arg(args, const char *)) != NULL)
          fprintf(file, "<th class=\"sortable\">%s</th>\n", col_name);
    fprintf(file, "</tr></thead>\n");

    return 0;
}

int table_row(void *pctx __attribute__ ((unused)), FILE * file, ...)
{
    va_list args;
    const char *val;

    va_start(args, file);

    fprintf(file, "<tr>\n");
    while ((val = va_arg(args, const char *)) != NULL)
          fprintf(file, "<td>%s</td>\n", val);
    fprintf(file, "</tr>\n");

    return 0;
}

int end_table(void *pctx __attribute__ ((unused)), FILE * file)
{
    fprintf(file, "</table>\n");
    return 0;
}

int generate_index_page(void *pctx, const char *filename)
{
    FILE *file;

    file = fopen(filename, "w");
    if (file == NULL)
        return -1;

    write_header(pctx, file, "Korean Amateur Database");

    fprintf(file, "<a href=\"players.html\">Player List</a><br/>\n");
    fprintf(file, "<a href=\"maps.html\">Map List</a><br/>\n");

    write_footer(pctx, file);

    return 0;
}

int generate_player_list(void *pctx, const char *filename)
{
    FILE *file;
    void *ctx;
    struct player_list_table_iter_args plti_args;

    file = fopen(filename, "w");

    ctx = talloc_new(pctx);
    if (ctx == NULL)
        goto failure;

    write_header(ctx, file, "Player List");

    start_table(ctx, file, "player_list", 2, true,
                "ID", "Race", "ELO", "ELO Peak", NULL);

    plti_args.file = file;
    plti_args.pctx = ctx;
    player_list_each(global_player_list, &player_list_table_iter, &plti_args);

    end_table(ctx, file);

    write_footer(ctx, file);

    fclose(file);
    return 0;

  failure:
    fclose(file);
    TALLOC_FREE(ctx);
    return 1;
}

int player_list_table_iter(struct player *player, void *args_uc)
{
    struct player_list_table_iter_args *args;
    const char *elo, *elo_peak;
    const char *player_link;
    void *ctx;

    args = args_uc;
    ctx = talloc_new(args->pctx);
    if (ctx == NULL)
        goto failure;

    elo = talloc_asprintf(ctx, "%d", (int)player_elo(player));
    elo_peak = talloc_asprintf(ctx, "%d", (int)player_elo_peak(player));
    player_link = talloc_asprintf(ctx,
                                  "<a href=\"player_%s.html\">%s</a>",
                                  player_key(player), player_id(player));
    table_row(ctx, args->file,
              player_link,
              race_string(player_race(player)), elo, elo_peak, NULL);

    TALLOC_FREE(ctx);
    return 0;

  failure:
    if (ctx != NULL)
        TALLOC_FREE(ctx);
    return 1;
}

int generate_player_page(struct player *player, void *args_uncast)
{
    struct generate_player_page_args *args;
    void *ctx;
    const char *file_name;
    const char *page_title;
    FILE *file;
    struct player_page_table_args ppt_args;

    args = args_uncast;
    ctx = talloc_new(args->pctx);
    if (ctx == NULL)
        return 1;

    file_name = talloc_asprintf(ctx, "%s/player_%s.html",
                                args->outdir, player_key(player));

    file = fopen(file_name, "w");
    if (file == NULL)
        goto failure;

    page_title = talloc_asprintf(ctx, "Player Page: %s\n", player_id(player));
    write_header(ctx, file, page_title);

    fprintf(file, "ID: <b>%s</b><br/>\n", player_id(player));
    fprintf(file, "Race: <b>%s</b><br/>\n", race_string(player_race(player)));
    fprintf(file, "Elo: <b>%d</b><br/>\n", (int)player_elo(player));
    fprintf(file, "Elo Peak: <b>%d</b><br/>\n", (int)player_elo_peak(player));
    fprintf(file, "Record: <b>%d</b> - <b>%d</b> (%.02f%%)<br/>\n",
            player_wins(player), player_losses(player),
            player_winrate(player) * 100);

    start_table(ctx, file, "game_list", 1, true,
                "Tournament", "Date", "Map", "Opponent", "Result", NULL);

    ppt_args.pctx = ctx;
    ppt_args.file = file;
    ppt_args.player_key = player_key(player);
    player_each_game(player, &player_page_table, &ppt_args);

    end_table(ctx, file);

    write_footer(ctx, file);

    fclose(file);
    TALLOC_FREE(ctx);
    return 0;

  failure:
    if (ctx != NULL)
        TALLOC_FREE(ctx);
    return 1;
}

int player_page_table(struct game *game, void *args_uncast)
{
    struct player_page_table_args *args;
    void *ctx;
    time_t game_time_int;
    struct tm game_time_tm;
    char game_time_str[LINE_MAX];
    const char *winner_key, *loser_key;
    const char *opponent_key, *opponent_link;
    struct player *opponent;
    const char *result;
    struct map *map;
    const char *map_link;

    args = args_uncast;
    ctx = talloc_new(args->pctx);
    if (ctx == NULL)
        return 1;

    /* Convert the game time to KST */
    game_time_int = game_time(game) + 32400;

    gmtime_r(&game_time_int, &game_time_tm);
    strftime(game_time_str, LINE_MAX, "%Y-%m-%d", &game_time_tm);

    winner_key = game_winner_key(game);
    loser_key = game_loser_key(game);

    opponent_key = (strcmp(winner_key, args->player_key) == 0)
        ? loser_key : winner_key;
    opponent = player_list_get(global_player_list, opponent_key);
    opponent_link =
        talloc_asprintf(ctx, "<a href=\"player_%s.html\">%s</a> (%s)",
                        opponent_key, player_id(opponent),
                        race_string(player_race(opponent)));

    if (opponent_link == NULL)
        goto failure;

    result = (strcmp(winner_key, args->player_key) == 0)
        ? "<b>win</b>" : "loss";

    map = map_list_get(global_map_list, game_map_key(game));
    map_link = talloc_asprintf(ctx, "<a href=\"map_%s.html\">%s</a>",
                               map_key(map), map_name(map));

    table_row(ctx, args->file, game_league_name(game), game_time_str,
              map_link, opponent_link, result, NULL);

    TALLOC_FREE(ctx);
    return 0;

  failure:
    if (ctx != NULL)
        TALLOC_FREE(ctx);
    return 1;
}

int generate_map_list(void *pctx, const char *filename)
{
    FILE *file;
    void *ctx;
    struct map_list_table_iter_args mlti_args;

    file = fopen(filename, "w");

    ctx = talloc_new(pctx);
    if (ctx == NULL)
        goto failure;

    write_header(ctx, file, "Map List");

    start_table(ctx, file, "map_list", 0, false, "Name", NULL);

    mlti_args.file = file;
    mlti_args.pctx = ctx;
    map_list_each(global_map_list, &map_list_table_iter, &mlti_args);

    end_table(ctx, file);

    write_footer(ctx, file);

    fclose(file);
    return 0;

  failure:
    fclose(file);
    TALLOC_FREE(ctx);
    return 1;
}

int map_list_table_iter(struct map *map, void *args_uc)
{
    struct map_list_table_iter_args *args;
    const char *map_link;
    void *ctx;

    args = args_uc;
    ctx = talloc_new(args->pctx);
    if (ctx == NULL)
        goto failure;

    map_link = talloc_asprintf(ctx, "<a href=\"map_%s.html\">%s</a>",
                               map_key(map), map_name(map));

    table_row(ctx, args->file, map_link, NULL);

    TALLOC_FREE(ctx);
    return 0;

  failure:
    if (ctx != NULL)
        TALLOC_FREE(ctx);
    return 1;
}

int generate_map_page(struct map *map, void *args_uncast)
{
    struct generate_map_page_args *args;
    void *ctx;
    const char *file_name;
    const char *page_title;
    FILE *file;
    struct map_page_table_args mpt_args;

    args = args_uncast;
    ctx = talloc_new(args->pctx);
    if (ctx == NULL)
        return 1;

    file_name = talloc_asprintf(ctx, "%s/map_%s.html",
                                args->outdir, map_key(map));

    file = fopen(file_name, "w");
    if (file == NULL)
        goto failure;

    page_title = talloc_asprintf(ctx, "Map Page: %s\n", map_name(map));
    write_header(ctx, file, page_title);

    fprintf(file, "Name: <b>%s</b><br/>\n", map_name(map));
    fprintf(file, "TvZ: <b>%d</b> - <b>%d</b> (%.02f%%)<br/>\n",
            map_tvz_wins(map), map_tvz_losses(map),
            map_tvz_winrate(map) * 100);
    fprintf(file, "ZvP: <b>%d</b> - <b>%d</b> (%.02f%%)<br/>\n",
            map_zvp_wins(map), map_zvp_losses(map),
            map_zvp_winrate(map) * 100);
    fprintf(file, "PvT: <b>%d</b> - <b>%d</b> (%.02f%%)<br/>\n",
            map_pvt_wins(map), map_pvt_losses(map),
            map_pvt_winrate(map) * 100);

    start_table(ctx, file, "game_list", 1, true,
                "Tournament", "Date", "Winner", "Loser", NULL);

    mpt_args.pctx = ctx;
    mpt_args.file = file;
    mpt_args.map_key = map_key(map);
    map_each_game(map, &map_page_table, &mpt_args);

    end_table(ctx, file);

    write_footer(ctx, file);

    fclose(file);
    TALLOC_FREE(ctx);
    return 0;

  failure:
    if (ctx != NULL)
        TALLOC_FREE(ctx);
    return 1;
}

int map_page_table(struct game *game, void *args_uncast)
{
    struct map_page_table_args *args;
    void *ctx;
    time_t game_time_int;
    struct tm game_time_tm;
    char game_time_str[LINE_MAX];
    const char *winner_key, *loser_key;
    struct player *winner, *loser;
    const char *winner_link, *loser_link;

    args = args_uncast;
    ctx = talloc_new(args->pctx);
    if (ctx == NULL)
        return 1;

    /* Convert the game time to KST */
    game_time_int = game_time(game) + 32400;

    gmtime_r(&game_time_int, &game_time_tm);
    strftime(game_time_str, LINE_MAX, "%Y-%m-%d", &game_time_tm);

    winner_key = game_winner_key(game);
    loser_key = game_loser_key(game);

    winner = player_list_get(global_player_list, winner_key);
    loser = player_list_get(global_player_list, loser_key);

    if (winner == NULL || loser == NULL)
        goto failure;

    winner_link = talloc_asprintf(ctx,
                                  "<a href=\"player_%s.html\">%s</a> (%s)",
                                  winner_key, player_id(winner),
                                  race_string(player_race(winner)));

    loser_link = talloc_asprintf(ctx,
                                 "<a href=\"player_%s.html\">%s</a> (%s)",
                                 loser_key, player_id(loser),
                                 race_string(player_race(loser)));

    if (winner_link == NULL || loser_link == NULL)
        goto failure;

    table_row(ctx, args->file,
              game_league_name(game),
              game_time_str, winner_link, loser_link, NULL);

    TALLOC_FREE(ctx);
    return 0;

  failure:
    if (ctx != NULL)
        TALLOC_FREE(ctx);
    return 1;
}
