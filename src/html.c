
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
#include "global.h"
#include "player.h"
#include "player_list.h"

#include <ftw.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <talloc.h>
#include <unistd.h>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/
struct player_list_table_iter_args
{
    FILE *file;
    void *pctx;
};

/***********************************************************************
 * Static Method Headers                                               *
 ***********************************************************************/
static int nftw_rm_rf(const char *path, const struct stat *sb,
                      int type, struct FTW *ftwbuf);

static int write_header(void *pctx, FILE * file, const char *page_name);
static int write_footer(void *pctx, FILE * file);

static int start_table(void *pctx, FILE * file, const char *id, ...);
static int table_row(void *pctx, FILE * file, ...);

static int generate_player_list(void *ctx, const char *filename);
static int player_list_table_iter(struct player *player, void *args_uc);

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
int html_generate(void *parent_context, const char *outdir)
{
    void *ctx;
    const char *player_list_filename;

    ctx = talloc_new(parent_context);
    if (ctx == NULL)
        return 1;

    /* This is an "rm -rf outdir", it cleans up everything in the
     * output directory */
    nftw(outdir, &nftw_rm_rf, 16, FTW_DEPTH);
    mkdir(outdir, 0777);

    /* Generates one page of every type. */
    player_list_filename = talloc_asprintf(ctx, "%s/players.html", outdir);
    generate_player_list(ctx, player_list_filename);

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
                FILE * file, const char *table_id, ...)
{
    va_list args;
    const char *col_name;

    va_start(args, table_id);

    fprintf(file, "<table id=\"%s\">\n", table_id);
    fprintf(file, "<tr>\n");
    while ((col_name = va_arg(args, const char *)) != NULL)
          fprintf(file, "<td><u>%s</u></td>\n", col_name);
    fprintf(file, "</tr>\n");

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

    start_table(ctx, file, "player_list",
                "ID", "Race", "ELO", "ELO Peak", NULL);

    plti_args.file = file;
    plti_args.pctx = ctx;
    player_list_each(global_player_list, &player_list_table_iter, &plti_args);

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
    void *ctx;

    args = args_uc;
    ctx = talloc_new(args->pctx);
    if (ctx == NULL)
        goto failure;

    elo = talloc_asprintf(ctx, "%d", (int)player_elo(player));
    elo_peak = talloc_asprintf(ctx, "%d", (int)player_elo_peak(player));
    table_row(ctx, args->file,
              player_id(player),
              race_string(player_race(player)), elo, elo_peak, NULL);

    TALLOC_FREE(ctx);
    return 0;

  failure:
    if (ctx != NULL)
        TALLOC_FREE(ctx);
    return 1;
}
