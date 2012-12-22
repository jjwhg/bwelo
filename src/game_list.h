
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

/* Creates an empty list of games */
struct game_list *game_list_new(void *c);

/* Adds the given game to the end of the list.  This checks that the
 * given game is newer than the newest game in the list, throwing an
 * error if it's not.  Returns 0 on success. */
int game_list_add(struct game_list *gl, struct game *g);

#endif