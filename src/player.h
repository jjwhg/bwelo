
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

#ifndef PLAYER_H
#define PLAYER_H

/* Stores all the information known about a single player. */
struct player;

#include "race.h"

typedef double player_elo_t;

/* Reads a player's information from a file, setting the remaining
 * information to the default values. */
struct player *player_read_file(void *c, const char *filename);

/* Records a win (and a loss for the other player) */
int player_win(struct player *winner, struct player *loser);

/* Access some basic data about a player. */
player_elo_t player_elo(struct player *player);
const char *player_name(struct player *player);
int player_wins(struct player *player);
int player_losses(struct player *player);

#endif
