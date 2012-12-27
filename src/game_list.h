
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

#ifndef GAME_LIST_H
#define GAME_LIST_H

struct game_list;

#include "game.h"
#include <stdint.h>

/* Allows for iteration along a list of games. */
struct game_list_iterator;

/* Creates an empty list of games */
struct game_list *game_list_new(void *c);

/* Adds the given game to the end of the list.  This checks that the
 * given game is newer than the newest game in the list, throwing an
 * error if it's not.  Returns 0 on success. */
int game_list_add(struct game_list *gl, struct game *g);

/* Creates a new game list iterator that points to the start of the list. */
struct game_list_iterator *game_list_iterator_new(void *c,
                                                  struct game_list *gl);

/* Returns the current (or NULL, if no current) element in the game
 * list this iterator is pointing to. */
struct game *game_list_iterator_cur(struct game_list_iterator *gli);

/* Moves the iterator to the next item. */
void game_list_iterator_next(struct game_list_iterator *gli);

/* Walks through each game in the list in no particular order. */
int game_list_each(struct game_list *gl,
                   int (*func) (struct game *, void *), void *arg);

#endif
