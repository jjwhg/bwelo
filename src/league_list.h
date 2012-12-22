
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

#ifndef LEAGUE_LIST_H
#define LEAGUE_LIST_H

/* Holds a list of all the leagues that have even been played. */
struct league_list;

#include "league.h"

/* Creates a new list of leagues by reading a directory full of league
 * files.  Every file in the directory should coorespond to a single
 * league.*/
struct league_list *league_list_new(void *ctx, const char *indir);

/* Adds a league to the given list of leagues.  Returns 0 on success. */
int league_list_add(struct league_list *ll, struct league *league);

#endif
