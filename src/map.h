
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

#ifndef MAP_H
#define MAP_H

/* Stores all the information known about a single map. */
struct map;

#include "game.h"

/* Reads a map's information from a file, setting the remaining
 * information to the default values. */
struct map *map_read_file(void *c, const char *filename, const char *key);

/* Adds a played game to the list of games this played has played */
int map_play(struct map *map, struct game *game);

/* Access some basic data about a map. */
const char *map_name(struct map *map);
const char *map_key(struct map *map);

/* Iterates through every game this map has played */
int map_each_game(struct map *map,
                  int (*iter) (struct game *, void *), void *data);

#endif
