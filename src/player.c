/*
    Woods - A sgame about crossing a stream
    Copyright (C) 2013 Justin Jacobs

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>

#include "SDL/SDL_ttf.h"
#include "math.h"

#include "player.h"
#include "sys.h"

const int WALK_DELAY = 16;
const int FRAME_TIME = 4;

Player player;

void playerInit() {
    player.x = 312;
    player.y = 72;
    player.frame = 0;
    player.frame_ticks = FRAME_TIME;
    player.walk_delay = WALK_DELAY;
    player.angle = (rand()%2) ? 1.0 : -1.0;
}

void playerLogic() {
    if (player.frame_ticks > 0) player.frame_ticks--;
    if (player.frame_ticks == 0) {
        player.frame_ticks = FRAME_TIME;
        (player.frame == 3) ? player.frame = 0 : player.frame++;
    }

    if (player.walk_delay > 0) player.walk_delay--;
    if (player.walk_delay == 0) {
        player.walk_delay = WALK_DELAY;
        player.y += 2;
    }

    player.angle += sin(player.angle * M_PI / 180.0)*1.5;

    if (player.angle > 80.0 || player.angle < -80.0) {
        trigger_game_over = true;
        return;
    }

    if (player.y >= 288) {
        trigger_game_over = true;
        victory = true;
    }
}

void playerMove() {
    if (action_cooldown > 0) return;

    if (action_left) {
        player.angle += 8;
        // shift weight left
    } else if (action_right) {
        player.angle -= 8;
        // shift weight right
    } else return;

    action_cooldown = ACTION_COOLDOWN;
}

