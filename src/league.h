
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

#ifndef LEAGUE_H
#define LEAGUE_H

/* Stores all the information known about a single league, including
 * every game played during the course of that league. */
struct league;

#include "game_list.h"

/* Reads a league's information from a file, setting the remaining
 * information to the default values. */
struct league *league_read_file(void *c, const char *filename);

/* Returns an iterator that iterates through every game in this league
 * in chronological order. */
struct game_list_iterator *league_game_iterator(struct league *l, void *c);

#endif
