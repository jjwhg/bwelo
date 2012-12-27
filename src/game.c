
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

#include "game.h"

#include <talloc.h>
#include <stdio.h>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/

/* Holds a single game.  Since games are stored inside a player they
 * sometimes can only reference the player key. */
struct game
{
    /* The key of the winner and loser, guarnteed not to be NULL */
    const char *winner_key;
    const char *loser_key;

    /* The map is also stored as a key, again to prevent needing to
     * have a loop in the dependency list. */
    const char *map_key;

    /* Each game sits inside a round and a group, mostly used for
     * display purposes */
    const char *league_name;
    const char *round;
    const char *group;

    /* The start time of the game.  Often times this isn't exact, but
     * it's used to order games for the Elo calculation. */
    game_time_t start_time;
};

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
int game_compare_time(const struct game *a, const struct game *b)
{
    if (a->start_time > b->start_time)
        return 1;
    else if (a->start_time < b->start_time)
        return -1;
    else
        return 0;
}

struct game *game_parse(void *ctx, const char *desc,
                        const char *league_name,
                        const char *round, const char *group)
{
    void *tmp;
    game_time_t start_date;
    char map_key[LINE_MAX];
    char player_1_key[LINE_MAX];
    char player_2_key[LINE_MAX];
    char winner;
    int count;
    const char *format;
    struct game *game;

    game = NULL;
    tmp = talloc_new(ctx);
    if (tmp == NULL)
        return NULL;

    /* The sscanf format string needs to be generated in order to
     * insert those LINE_MAX width specifiers to avoid buffer
     * overflows. */
    format = talloc_asprintf(tmp, "%%ld %%%ds %%%ds %%1c %%%ds",
                             LINE_MAX, LINE_MAX, LINE_MAX);
    if (format == NULL)
        goto failure;
    count = sscanf(desc, format, &start_date, map_key, player_1_key,
                   &winner, player_2_key);
    if (count != 5)
        goto failure;

    /* Fills out a new game. */
    game = talloc(ctx, struct game);
    if (game == NULL)
        goto failure;

    game->start_time = start_date;
    game->league_name = talloc_reference(game, league_name);
    game->map_key = talloc_strdup(game, map_key);
    game->round = talloc_reference(game, round);

    if (group != NULL)
        game->group = talloc_reference(game, group);
    else
        game->group = NULL;

    if (winner == '>')
    {
        game->winner_key = talloc_strdup(game, player_1_key);
        game->loser_key = talloc_strdup(game, player_2_key);
    }
    else if (winner == '<')
    {
        game->winner_key = talloc_strdup(game, player_2_key);
        game->loser_key = talloc_strdup(game, player_1_key);
    }
    else
        goto failure;

    TALLOC_FREE(tmp);
    return game;

  failure:
    if (game != NULL)
        TALLOC_FREE(game);

    TALLOC_FREE(tmp);
    return NULL;
}

const char *game_winner_key(struct game *game)
{
    return game->winner_key;
}

const char *game_loser_key(struct game *game)
{
    return game->loser_key;
}

game_time_t game_time(struct game * game)
{
    return game->start_time;
}

const char *game_league_name(struct game *game)
{
    return game->league_name;
}

const char *game_map_key(struct game *game)
{
    return game->map_key;
}
