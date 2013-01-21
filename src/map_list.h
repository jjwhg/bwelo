
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

#ifndef MAP_LIST_H
#define MAP_LIST_H

/* Stores a list of maps.  This is purposely opaque. */
struct map_list;

#include "map.h"

/* Creates a new list of maps, given an input directory.  Every file
 * in the input directory will coorespond to a single map: the file
 * name is its key while the contents of the file describes the map.
 */
struct map_list *map_list_new(void *context, const char *indir);

/* Associates a given key with a given map.  Returns 0 on success. */
int map_list_add(struct map_list *ml, const char *key, struct map *map);

/* Copies a single copy designated by the given key to the target
 * list.  This is a SHALLOW copy, so the map isn't duplicated, it's
 * just in two lists. */
int map_list_copy(struct map_list *ml, const char *key,
                  struct map_list *target);

/* Looks up the map designated by the given key in a list. */
struct map *map_list_get(struct map_list *ml, const char *key);

/* Walks through each map in the list in no particular order. */
int map_list_each(struct map_list *ml,
                  int (*func) (struct map *, void *), void *arg);

#endif
