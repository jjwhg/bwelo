
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

#include "player_list.h"
#include "league_list.h"
#include "global.h"
#include "html.h"

#include <stdio.h>
#include <talloc.h>

/***********************************************************************
 * Static Method Headers                                               *
 ***********************************************************************/
static int print_elo(struct player *player, void *unused);
static int update_elo(struct game *game, void *unused);

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
int main(int argc, char **argv)
{
    void *root_context;
    struct league_list *league_list;

    /* Parse commandline arguments. */
    {
        int i;

        for (i = 1; i < argc; i++)
            fprintf(stderr, "Unknown argument: '%s'\n", argv[i]);

        if (i > 1)
        {
            fprintf(stderr, "Hint, %s takes no arguments\n", argv[0]);
            return 1;
        }
    }

    /* Create an empty root context. */
    root_context = talloc_new(NULL);

    /* Initialize the list of players, leagues, and games. */
    global_player_list = player_list_new(root_context, INDIR "/players");
    global_map_list = map_list_new(root_context, INDIR "/maps");
    league_list = league_list_new(root_context, INDIR "/leagues");

    /* Generates each player's Elo rating */
    league_list_each_game(league_list, &update_elo, NULL);

    /* List every player's Elo rating to stdout */
    player_list_each(global_player_list, &print_elo, NULL);

    /* Open up a new HTML generator */
    if (html_generate(root_context, OUTDIR) != 0)
        fprintf(stderr, "HTML generation failed\n");

    /* Clean up everything we've allocated. */
    TALLOC_FREE(root_context);

    return 0;
}

/***********************************************************************
 * Static Methods                                                      *
 ***********************************************************************/
int print_elo(struct player *player, void *uu __attribute__ ((unused)))
{
    printf("%4d (%2d-%2d) %s\n", (int)player_elo(player),
           player_wins(player), player_losses(player), player_id(player));
    return 0;
}

int update_elo(struct game *game, void *uu __attribute__ ((unused)))
{
    const char *winner_key, *loser_key;
    struct player *winner, *loser;
    const char *map_key;
    struct map *map;

    winner_key = game_winner_key(game);
    loser_key = game_loser_key(game);
    if (winner_key == NULL || loser_key == NULL)
        return -1;

    winner = player_list_get(global_player_list, winner_key);
    loser = player_list_get(global_player_list, loser_key);
    if (winner == NULL || loser == NULL)
        return -1;

    if (player_win(winner, loser) != 0)
        return -1;

    player_play(winner, game);
    player_play(loser, game);

    map_key = game_map_key(game);
    map = map_list_get(global_map_list, map_key);
    map_play(map, game);

    return 0;
}
