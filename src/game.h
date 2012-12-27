
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

#ifndef GAME_H
#define GAME_H

#include <stdint.h>

struct game;

/* Game times are stored as UNIX time, in seconds, in this format.  -1
 * means an unknown time. */
typedef int64_t game_time_t;

/* Compares the time of two games, returning 0 if they're the same, 1
 * if a is newer than b, and -1 if b is newer than a. */
int game_compare_time(const struct game *a, const struct game *b);

/* Parses a game given a string read directly from the game listing
 * file.  This string should have the "GAME " part stripped
 * already. */
struct game *game_parse(void *ctx, const char *desc,
                        const char *league_name,
                        const char *round, const char *group);

/* Returns (as a key) the winner/loser of a given game.  This will be
 * a pointer into the game structure, so don't expect it to exist for
 * long -- reference if you want it to stick around. */
const char *game_winner_key(struct game *game);
const char *game_loser_key(struct game *game);
game_time_t game_time(struct game *game);
const char *game_league_name(struct game *game);
const char *game_map_key(struct game *game);

#endif
