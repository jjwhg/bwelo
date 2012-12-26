
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

#include "player_list.h"

#include <dirent.h>
#include <talloc.h>
#include <string.h>

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/

/* Holds a link to a single player along with its key. */
struct player_list_node
{
    const char *key;
    struct player_list_node *next;
    struct player *data;
};

/* Just points to the list. */
struct player_list
{
    struct player_list_node *head;
};

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
struct player_list *player_list_new(void *context, const char *indir)
{
    struct player_list *pl;
    /* Used by the call to scandir */
    struct dirent **namelist;
    size_t dir_entries;
    size_t i;
    /* Temporary memory */
    void *tcxt;

    pl = talloc(context, struct player_list);
    if (pl == NULL)
        return NULL;

    tcxt = talloc_new(pl);

    /* Start off with no players. */
    pl->head = NULL;

    /* If there's no given input directory then just make an empty
     * list */
    if (indir == NULL)
        goto success;

    /* Read the entire directory we've been passed. */
    dir_entries = scandir(indir, &namelist, NULL, NULL);

    for (i = 0; i < dir_entries; i++)
    {
        const char *key;
        const char *player_filename;
        struct player *player;

        /* The key isn't part of talloc, we need it imported. */
        key = talloc_strdup(tcxt, namelist[i]->d_name);
        free(namelist[i]);

        /* Skip hidden files -- this could be done with a scandir()
         * filter, but I'm lazy! */
        if (key[0] == '.')
            continue;

        /* Read the player information from the given file. */
        player_filename = talloc_asprintf(tcxt, "%s/%s", indir, key);
        player = player_read_file(tcxt, player_filename);
        if (player == NULL)
            goto failure;

        /* Adds the player to this list. */
        if (player_list_add(pl, key, player) != 0)
            goto failure;
    }

  success:
    TALLOC_FREE(tcxt);
    return pl;

  failure:
    TALLOC_FREE(tcxt);
    TALLOC_FREE(pl);
    return NULL;
}

int player_list_add(struct player_list *pl, const char *key,
                    struct player *player)
{
    struct player_list_node *new;

    new = talloc(pl, struct player_list_node);
    if (new == NULL)
        return -1;

    /* Simply adds to the head of the list without any checking at all! */
    new->next = pl->head;
    new->key = talloc_reference(new, key);
    new->data = talloc_reference(new, player);
    pl->head = new;
    return 0;
}

int player_list_copy(struct player_list *pl, const char *key,
                     struct player_list *target)
{
    struct player *found;

    found = player_list_get(pl, key);

    /* It's impossible to copy a player that doesn't exist. */
    if (found == NULL)
        return -1;

    return player_list_add(target, key, found);
}

struct player *player_list_get(struct player_list *pl, const char *key)
{
    struct player_list_node *cur;

    cur = pl->head;
    while (cur != NULL)
    {
        if (strcmp(cur->key, key) == 0)
            return cur->data;

        cur = cur->next;
    }

    return NULL;
}

int player_list_each(struct player_list *pl,
                     int (*func) (struct player *, void *), void *arg)
{
    struct player_list_node *cur;

    cur = pl->head;
    while (cur != NULL)
    {
        int ret;

        if ((ret = func(cur->data, arg)) != 0)
            return ret;

        cur = cur->next;
    }

    return 0;
}
