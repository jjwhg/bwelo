
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

#ifndef PLAYER_LIST_H
#define PLAYER_LIST_H

/* Stores a list of players.  This is purposely opaque. */
struct player_list;

#include "player.h"

/* Creates a new list of players, given an input directory.  Every
 * file in the input directory will coorespond to a single player: the
 * file name is its key while the contents of the file describes the
 * player.
 */
struct player_list *player_list_new(void *context, const char *indir);

/* Associates a given key with a given player.  Returns 0 on success. */
int player_list_add(struct player_list *pl, const char *key,
                    struct player *player);

/* Copies a single player designated by the given key to the target
 * list.  This is a SHALLOW copy, so the player isn't duplicated, it's
 * just in two lists. */
int player_list_copy(struct player_list *pl, const char *key,
                     struct player_list *target);

/* Looks up the player designated by the given key in a list. */
struct player *player_list_get(struct player_list *pl, const char *key);

/* Walks through each player in the list in no particular order. */
int player_list_each(struct player_list *pl,
                     int (*func) (struct player *, void *), void *arg);

#endif
