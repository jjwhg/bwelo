
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

#define _BSD_SOURCE

#include "league_list.h"
#include "league.h"

#include <dirent.h>
#include <talloc.h>

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/

/* Holds a link to a single league */
struct league_list_node
{
    struct league_list_node *next;
    struct league *data;
};

/* Just points to the list */
struct league_list
{
    struct league_list_node *head;
};

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
struct league_list *league_list_new(void *context, const char *indir)
{
    struct league_list *ll;
    /* Used by the call to scandir */
    struct dirent **namelist;
    size_t dir_entries;
    size_t i;
    /* Temporary memory */
    void *tcxt;

    ll = talloc(context, struct league_list);
    if (ll == NULL)
        return NULL;

    tcxt = talloc_new(ll);

    /* Start off with no leagues. */
    ll->head = NULL;

    /* Read the entire directory we've been passed. */
    dir_entries = scandir(indir, &namelist, NULL, NULL);

    for (i = 0; i < dir_entries; i++)
    {
        const char *league_name;
        const char *league_filename;
        struct league *league;

        /* Store a copy of the league name for later use */
        league_name = talloc_strdup(tcxt, namelist[i]->d_name);
        free(namelist[i]);

        /* Skip hidden files -- this could be done with a scandir()
         * filter, but I'm lazy! */
        if (league_name[0] == '.')
            continue;

        /* Read the league information from the given file  */
        league_filename = talloc_asprintf(tcxt, "%s/%s", indir, league_name);
        league = league_read_file(tcxt, league_filename);

        /* Adds the league to this list. */
        if (league_list_add(ll, league) != 0)
        {
            fprintf(stderr, "Failed to read league '%s'\n", league_name);
            goto failure;
        }
    }

    TALLOC_FREE(tcxt);
    return ll;

  failure:
    TALLOC_FREE(tcxt);
    TALLOC_FREE(ll);
    return NULL;
}

int league_list_add(struct league_list *ll, struct league *league)
{
    struct league_list_node *new;

    new = talloc(ll, struct league_list_node);
    if (new == NULL)
        return -1;

    /* Simply adds to the head of the list without any checking at all! */
    new->next = ll->head;
    new->data = talloc_reference(new, league);
    ll->head = new;
    return 0;
}

int league_list_each_game(struct league_list *ll,
                          int (*iter) (struct game *, void *), void *data)
{
    size_t league_count;
    void *ctx;
    struct game_list_iterator **iters;

    ctx = talloc_new(ll);
    if (ctx == NULL)
        return -1;

    /* Count the number of leagues in this list */
    {
        struct league_list_node *cur;

        league_count = 0;
        cur = ll->head;
        while (cur != NULL)
        {
            league_count++;
            cur = cur->next;
        }
    }

    /* Create a new game_list_iterator for every league */
    iters = talloc_array(ctx, struct game_list_iterator *, league_count);
    {
        struct league_list_node *cur;
        size_t i;

        i = 0;
        cur = ll->head;
        while (cur != NULL)
        {
            iters[i] = league_game_iterator(cur->data, iters);
            i++;
            cur = cur->next;
        }
    }

    /* Sorts the games from different legaues into a total ordering */
    {
        struct game *game;
        struct game_list_iterator *game_iter;

        do
        {
            size_t i;

            /* Start out without having had found a game during this
             * iteration -- this is how we break the loop */
            game = NULL;

            /* Find the oldest remaining game. */
            for (i = 0; i < league_count; i++)
            {
                struct game *ngame;

                /* Look at the game suggested by the given league */
                ngame = game_list_iterator_cur(iters[i]);
                if (ngame == NULL)
                    continue;

                /* If the current league's game is older than the game
                 * we would otherwise spit out, then use the older
                 * game */
                if ((game == NULL) || (game_compare_time(game, ngame) > 0))
                {
                    game = ngame;
                    game_iter = iters[i];
                }
            }

            /* If any game was found then pass it back to the user and
             * remove it from consideration */
            if (game != NULL)
            {
                int ret;

                if ((ret = iter(game, data)) != 0)
                {
                    TALLOC_FREE(ctx);
                    return ret;
                }
                game_list_iterator_next(game_iter);
            }
        } while (game != NULL);
    }

    TALLOC_FREE(ctx);
    return 0;
}
