
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

#include "league.h"
#include "game_list.h"
#include "player_list.h"
#include "global.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <talloc.h>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/
struct league
{
    const char *name;

    /* Lists every player in this league. */
    struct player_list *players;
};

/***********************************************************************
 * Static Method Headers                                               *
 ***********************************************************************/

/* Returns TRUE if the haystack starts with the needle. */
static bool string_starts_with(const char *haystack, const char *needle);

/* Returts NULL if the haystack doesn't start with the needle.
 * Otherwise, removes the needle from the start of the hackstack and
 * strips any leading whitespace.  Note that no memory is allocated
 * here, it's just pointer arithmetic. */
static const char *strip_front(const char *hs, const char *ne);

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
struct league *league_read_file(void *c, const char *filename)
{
    struct league *l;
    FILE *lf;
    char buf[LINE_MAX];
    void *tmp;
    const char *round;
    const char *group;
    int line_number;

    l = talloc(c, struct league);
    if (l == NULL)
        return NULL;

    /* Sets everything to the default. */
    line_number = 1;
    tmp = talloc_new(l);
    round = NULL;
    group = NULL;
    l->name = NULL;
    l->players = player_list_new(l, NULL);

    /* Reads the input file. */
    lf = fopen(filename, "r");
    if (lf == NULL)
        goto error;

    while (fgets(buf, LINE_MAX, lf) != NULL)
    {
        const char *b;

        /* Remove avy trailing whitespace, this will be at least every
         * newline. */
        while (strlen(buf) > 0 && isspace(buf[strlen(buf) - 1]))
            buf[strlen(buf) - 1] = '\0';

        /* Attempt to parse! */
        if ((b = strip_front(buf, "NAME ")) != NULL)
            l->name = talloc_strdup(l, b);
        else if ((b = strip_front(buf, "PLAYER ")) != NULL)
        {
            const char *k;

            k = talloc_strdup(tmp, b);
            if (player_list_copy(global_player_list, k, l->players) != 0)
                goto error;
        }
        else if ((b = strip_front(buf, "ROUND ")) != NULL)
        {
            round = talloc_strdup(tmp, b);
            group = NULL;
        }
        else if ((b = strip_front(buf, "GROUP ")) != NULL)
            group = talloc_strdup(tmp, b);
        else if ((b = strip_front(buf, "GAME ")) != NULL)
        {
            struct game *game;
            const char *winner_key, *loser_key;
            struct player *winner, *loser;

            game = game_parse(tmp, b, round, group);
            if (game == NULL)
            {
                fprintf(stderr, "%s:%d Unable to parse game data\n",
                        filename, line_number);
                goto error;
            }

            winner_key = game_winner_key(game);
            loser_key = game_loser_key(game);
            if (winner_key == NULL || loser_key == NULL)
                goto error;

            winner = player_list_get(l->players, winner_key);
            loser = player_list_get(l->players, loser_key);
            if (winner == NULL || loser == NULL)
                goto error;

            if (player_win(winner, loser) != 0)
                goto error;
        }

        line_number++;
    }

    TALLOC_FREE(tmp);
    fclose(lf);
    lf = NULL;
    return l;

  error:
    fprintf(stderr, "league.c: die %s %d\n", filename, line_number);

    if (lf != NULL)
        fclose(lf);

    TALLOC_FREE(tmp);
    TALLOC_FREE(l);
    return NULL;
}

/***********************************************************************
 * Static Methods                                                      *
 ***********************************************************************/
bool string_starts_with(const char *haystack, const char *needle)
{
    return (strncmp(haystack, needle, strlen(needle)) == 0);
}

const char *strip_front(const char *haystack, const char *needle)
{
    if (!string_starts_with(haystack, needle))
        return false;

    haystack += strlen(needle);

    while (isspace(*haystack))
        haystack++;

    return haystack;
}
