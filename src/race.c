
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

#include "race.h"

#include <stdlib.h>
#include <strings.h>

/***********************************************************************
 * Extern Methods                                                      *
 ***********************************************************************/
enum race race_parse(const char *str)
{
    if (strcasecmp(str, "terran") == 0)
        return RACE_TERRAN;
    if (strcasecmp(str, "zerg") == 0)
        return RACE_ZERG;
    if (strcasecmp(str, "protoss") == 0)
        return RACE_PROTOSS;

    return RACE_UNKNOWN;
}

const char *race_string(enum race race)
{
    switch (race)
    {
    case RACE_UNKNOWN:
        return "Unknown";
    case RACE_TERRAN:
        return "Terran";
    case RACE_ZERG:
        return "Zerg";
    case RACE_PROTOSS:
        return "Protoss";
    case RACE_RANDOM:
        return "Random";
    }

    /* FIXME: Can't get here leagally, but it's still necessary
     *        because I guess you could cast an integer to the enum. */
    return NULL;
}
