
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

#ifndef RACE_H
#define RACE_H

/* There are 4 races in Brood War, plus an uninitialized value */
enum race
{
    RACE_UNKNOWN,
    RACE_TERRAN,
    RACE_ZERG,
    RACE_PROTOSS,
    RACE_RANDOM,
};

/* Parses a string into a race. */
enum race race_parse(const char *str);

#endif
