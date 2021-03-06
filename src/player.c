
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

#include "player.h"
#include "game_list.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <talloc.h>
#include <math.h>

#ifndef PLAYER_DEFAULT_ELO
#define PLAYER_DEFAULT_ELO 2000
#endif

#ifndef ELO_K1
#define ELO_K1 30
#endif

#ifndef ELO_K2
#define ELO_K2 15
#endif

#ifndef ELO_K3
#define ELO_K3 10
#endif

#ifndef ELO_K1_GAMES
#define ELO_K1_GAMES 30
#endif

#ifndef ELO_K2_GAMES
#define ELO_K2_GAMES 100
#endif

#ifndef ELO_PEAK_GAMES
#define ELO_PEAK_GAMES 10
#endif

#ifndef LINE_MAX
#define LINE_MAX 1024
#endif

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/
struct player
{
    /* The player's well-formatted ID */
    const char *id;

    /* The race this player plays most often. */
    enum race race;

    /* A list of every game this player has played. */
    struct game_list *games;

    /* Calculates a running Elo rating. */
    player_elo_t elo;
    player_elo_t peak_elo;

    /* The win-loss ratio of this player */
    int wins;
    int losses;

    /* The key that uniquely identifies this player */
    const char *key;
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
struct player *player_read_file(void *c, const char *filename,
                                const char *key)
{
    struct player *p;
    FILE *pf;
    char buf[LINE_MAX];

    p = talloc(c, struct player);
    if (p == NULL)
        return NULL;

    /* Sets everything to the default. */
    p->id = NULL;
    p->race = RACE_UNKNOWN;
    p->games = game_list_new(p);
    p->elo = PLAYER_DEFAULT_ELO;
    p->peak_elo = 0;
    p->wins = 0;
    p->losses = 0;
    p->key = NULL;

    /* There should be a unique key, but apparently sometimes there's
     * not. */
    if (key != NULL)
        p->key = talloc_reference(p, key);

    /* Reads the input file. */
    pf = fopen(filename, "r");
    if (pf == NULL)
        goto error;

    while (fgets(buf, LINE_MAX, pf) != NULL)
    {
        const char *b;

        /* Remove avy trailing whitespace, this will be at least every
         * newline. */
        while (strlen(buf) > 0 && isspace(buf[strlen(buf) - 1]))
            buf[strlen(buf) - 1] = '\0';

        /* Attempt to parse! */
        if ((b = strip_front(buf, "ID ")) != NULL)
            p->id = talloc_strdup(p, b);
        else if ((b = strip_front(buf, "RACE ")) != NULL)
            p->race = race_parse(b);
    }

    fclose(pf);
    pf = NULL;
    return p;

  error:
    if (pf != NULL)
        fclose(pf);

    TALLOC_FREE(p);
    return NULL;
}

int player_win(struct player *winner, struct player *loser)
{
    player_elo_t q_w, q_l;
    player_elo_t e_w, e_l;
    int k_w, k_l;
    int g_w, g_l;

    q_w = pow(10.0, winner->elo / 400.0);
    q_l = pow(10.0, loser->elo / 400.0);

    e_w = q_w / (q_w + q_l);
    e_l = q_l / (q_l + q_w);

    g_w = winner->wins + winner->losses;
    g_l = loser->wins + loser->losses;

    k_w = ELO_K1;
    if (g_w > ELO_K1_GAMES)
        k_w = ELO_K2;
    if (g_w > ELO_K2_GAMES)
        k_w = ELO_K3;

    k_l = ELO_K1;
    if (g_l > ELO_K1_GAMES)
        k_l = ELO_K2;
    if (g_l > ELO_K2_GAMES)
        k_l = ELO_K3;

    winner->elo += k_w * (1.0 - e_w);
    loser->elo += k_l * (0.0 - e_l);

    if (g_w > ELO_PEAK_GAMES)
        if (winner->elo > winner->peak_elo)
            winner->peak_elo = winner->elo;

    if (g_l > ELO_PEAK_GAMES)
        if (loser->elo > loser->peak_elo)
            loser->peak_elo = loser->elo;

    winner->wins++;
    loser->losses++;

    return 0;
}

int player_play(struct player *player, struct game *game)
{
    return game_list_add(player->games, game);
}

player_elo_t player_elo(struct player * player)
{
    return player->elo;
}

player_elo_t player_elo_peak(struct player * player)
{
    return player->peak_elo;
}

const char *player_id(struct player *player)
{
    return player->id;
}

int player_wins(struct player *player)
{
    return player->wins;
}

int player_losses(struct player *player)
{
    return player->losses;
}

double player_winrate(struct player *player)
{
    return player->wins / (double)(player->wins + player->losses);
}

enum race player_race(struct player *player)
{
    return player->race;
}

const char *player_key(struct player *player)
{
    return player->key;
}

int player_each_game(struct player *player,
                     int (*iter) (struct game *, void *), void *data)
{
    return game_list_each(player->games, iter, data);
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
