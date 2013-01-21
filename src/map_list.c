
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

#include "map_list.h"

#include <dirent.h>
#include <talloc.h>
#include <string.h>

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/

/* Holds a link to a single map along with its key. */
struct map_list_node
{
    const char *key;
    struct map_list_node *next;
    struct map *data;
};

/* Just points to the list. */
struct map_list
{
    struct map_list_node *head;
};

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
struct map_list *map_list_new(void *context, const char *indir)
{
    struct map_list *ml;
    /* Used by the call to scandir */
    struct dirent **namelist;
    size_t dir_entries;
    size_t i;
    /* Temporary memory */
    void *tcxt;

    ml = talloc(context, struct map_list);
    if (ml == NULL)
        return NULL;

    tcxt = talloc_new(ml);

    /* Start off with no maps. */
    ml->head = NULL;

    /* If there's no given input directory then just make an empty
     * list */
    if (indir == NULL)
        goto success;

    /* Read the entire directory we've been passed. */
    dir_entries = scandir(indir, &namelist, NULL, NULL);

    for (i = 0; i < dir_entries; i++)
    {
        const char *key;
        const char *map_filename;
        struct map *map;

        /* The key isn't part of talloc, we need it imported. */
        key = talloc_strdup(tcxt, namelist[i]->d_name);
        free(namelist[i]);

        /* Skip hidden files -- this could be done with a scandir()
         * filter, but I'm lazy! */
        if (key[0] == '.')
            continue;

        /* Read the map information from the given file. */
        map_filename = talloc_asprintf(tcxt, "%s/%s", indir, key);
        map = map_read_file(tcxt, map_filename, key);
        if (map == NULL)
            goto failure;

        /* Adds the map to this list. */
        if (map_list_add(ml, key, map) != 0)
            goto failure;
    }

  success:
    TALLOC_FREE(tcxt);
    return ml;

  failure:
    TALLOC_FREE(tcxt);
    TALLOC_FREE(ml);
    return NULL;
}

int map_list_add(struct map_list *ml, const char *key, struct map *map)
{
    struct map_list_node *new;

    new = talloc(ml, struct map_list_node);
    if (new == NULL)
        return -1;

    /* Simply adds to the head of the list without any checking at all! */
    new->next = ml->head;
    new->key = talloc_reference(new, key);
    new->data = talloc_reference(new, map);
    ml->head = new;
    return 0;
}

int map_list_copy(struct map_list *ml, const char *key,
                  struct map_list *target)
{
    struct map *found;

    found = map_list_get(ml, key);

    /* It's impossible to copy a map that doesn't exist. */
    if (found == NULL)
        return -1;

    return map_list_add(target, key, found);
}

struct map *map_list_get(struct map_list *ml, const char *key)
{
    struct map_list_node *cur;

    cur = ml->head;
    while (cur != NULL)
    {
        if (strcmp(cur->key, key) == 0)
            return cur->data;

        cur = cur->next;
    }

    return NULL;
}

int map_list_each(struct map_list *ml,
                  int (*func) (struct map *, void *), void *arg)
{
    struct map_list_node *cur;

    cur = ml->head;
    while (cur != NULL)
    {
        int ret;

        if ((ret = func(cur->data, arg)) != 0)
            return ret;

        cur = cur->next;
    }

    return 0;
}
