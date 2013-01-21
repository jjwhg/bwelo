
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

#include "map.h"
#include "game_list.h"
#include "global.h"
#include "player_list.h"
#include "race.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <talloc.h>
#include <math.h>

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/
struct map
{
    /* The English name of this map. */
    const char *name;

    /* A list of every game this map has played. */
    struct game_list *games;

    /* The key that uniquely identifies this map */
    const char *key;

    /* Calculates the winrates for each game type. */
    int zvp_wins, zvp_losses;
    int pvt_wins, pvt_losses;
    int tvz_wins, tvz_losses;
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
struct map *map_read_file(void *c, const char *filename, const char *key)
{
    struct map *m;
    FILE *mf;
    char buf[LINE_MAX];

    m = talloc(c, struct map);
    if (m == NULL)
        return NULL;

    /* Sets everything to the default. */
    m->name = NULL;
    m->games = game_list_new(m);
    m->key = NULL;
    m->zvp_wins = m->zvp_losses = 0;
    m->pvt_wins = m->pvt_losses = 0;
    m->tvz_wins = m->tvz_losses = 0;

    /* There should be a unique key, but apparently sometimes there's
     * not. */
    if (key != NULL)
        m->key = talloc_reference(m, key);

    /* Reads the input file. */
    mf = fopen(filename, "r");
    if (mf == NULL)
        goto error;

    while (fgets(buf, LINE_MAX, mf) != NULL)
    {
        const char *b;

        /* Remove avy trailing whitespace, this will be at least every
         * newline. */
        while (strlen(buf) > 0 && isspace(buf[strlen(buf) - 1]))
            buf[strlen(buf) - 1] = '\0';

        /* Attempt to parse! */
        if ((b = strip_front(buf, "NAME ")) != NULL)
            m->name = talloc_strdup(m, b);
    }

    fclose(mf);
    mf = NULL;
    return m;

  error:
    if (mf != NULL)
        fclose(mf);

    TALLOC_FREE(m);
    return NULL;
}

int map_play(struct map *map, struct game *game)
{
    struct player *winner, *loser;
    enum race winner_race, loser_race;

    winner = player_list_get(global_player_list, game_winner_key(game));
    loser = player_list_get(global_player_list, game_loser_key(game));

    winner_race = player_race(winner);
    loser_race = player_race(loser);

    if (winner_race == RACE_ZERG && loser_race == RACE_PROTOSS)
        map->zvp_wins++;
    if (winner_race == RACE_PROTOSS && loser_race == RACE_TERRAN)
        map->pvt_wins++;
    if (winner_race == RACE_TERRAN && loser_race == RACE_ZERG)
        map->tvz_wins++;
    if (loser_race == RACE_ZERG && winner_race == RACE_PROTOSS)
        map->zvp_losses++;
    if (loser_race == RACE_PROTOSS && winner_race == RACE_TERRAN)
        map->pvt_losses++;
    if (loser_race == RACE_TERRAN && winner_race == RACE_ZERG)
        map->tvz_losses++;

    return game_list_add(map->games, game);
}

const char *map_name(struct map *map)
{
    return map->name;
}

const char *map_key(struct map *map)
{
    return map->key;
}

int map_zvp_wins(struct map *map)
{
    return map->zvp_wins;
}

int map_pvt_wins(struct map *map)
{
    return map->pvt_wins;
}

int map_tvz_wins(struct map *map)
{
    return map->tvz_wins;
}

int map_zvp_losses(struct map *map)
{
    return map->zvp_losses;
}

int map_pvt_losses(struct map *map)
{
    return map->pvt_losses;
}

int map_tvz_losses(struct map *map)
{
    return map->tvz_losses;
}

double map_zvp_winrate(struct map *map)
{
    return map->zvp_wins / (double)(map->zvp_wins + map->zvp_losses);
}

double map_pvt_winrate(struct map *map)
{
    return map->pvt_wins / (double)(map->pvt_wins + map->pvt_losses);
}

double map_tvz_winrate(struct map *map)
{
    return map->tvz_wins / (double)(map->tvz_wins + map->tvz_losses);
}

int map_each_game(struct map *map,
                  int (*iter) (struct game *, void *), void *data)
{
    return game_list_each(map->games, iter, data);
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
