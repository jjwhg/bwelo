
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

#include "game_list.h"

#include <talloc.h>

/***********************************************************************
 * Structures                                                          *
 ***********************************************************************/

/* Holds an ordered list of games */
struct game_list_node
{
    struct game_list_node *next;
    struct game *data;
};

/* Just points to the list */
struct game_list
{
    struct game_list_node *head;
    struct game_list_node *tail;
};

/* Allows for iteration along a list of games. */
struct game_list_iterator
{
    struct game_list_node *cur;
};

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
struct game_list *game_list_new(void *ctx)
{
    struct game_list *gl;

    gl = talloc(ctx, struct game_list);
    if (gl == NULL)
        return NULL;

    gl->head = NULL;
    gl->tail = NULL;

    return gl;
}

int game_list_add(struct game_list *gl, struct game *g)
{
    struct game_list_node *new;

    new = talloc(gl, struct game_list_node);
    new->next = NULL;
    new->data = talloc_reference(new, g);

    /* Special case for an empty list */
    if (gl->tail == NULL)
    {
        gl->head = gl->tail = new;
        return 0;
    }

    /* Ensure the games are all added in order */
    if (game_compare_time(gl->tail->data, new->data) != -1)
    {
        TALLOC_FREE(new);
        return -1;
    }

    gl->tail->next = new;
    gl->tail = new;
    return 0;
}

struct game_list_iterator *game_list_iterator_new(void *c,
                                                  struct game_list *gl)
{
    struct game_list_iterator *gli;

    gli = talloc(c, struct game_list_iterator);
    if (gli == NULL)
        return NULL;

    gli->cur = gl->head;

    return gli;
}

struct game *game_list_iterator_cur(struct game_list_iterator *gli)
{
    if (gli->cur == NULL)
        return NULL;

    return gli->cur->data;
}

void game_list_iterator_next(struct game_list_iterator *gli)
{
    if (gli->cur == NULL)
        return;

    gli->cur = gli->cur->next;
}
