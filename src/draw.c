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

#include "SDL/SDL_ttf.h"

#include "SDL_rotozoom.h"

#include "draw.h"
#include "menu.h"
#include "player.h"
#include "sys.h"

void drawEverything() {
    // Fill the screen with black
    SDL_FillRect(screen,NULL, 0x000000);

    SDL_BlitSurface(surface_background,NULL,screen,NULL);

    if (title_screen) {
        drawTitle();
    } else if (options_screen > -1) {
        drawOptions();
    } else {
        drawInfo();
        drawPlayer();
    }
}

void drawMenu(int offset) {
    SDL_Surface *text;
    SDL_Color color = {217,217,217};
    SDL_Rect dest;
    int i;

    for (i=0;i<menu_size;i++) {
        dest.x = 0;
        dest.y = SCREEN_HEIGHT - ((menu_size-i) * surface_bar->h) - offset;
        if (i == menu_option) SDL_BlitSurface(surface_bar,NULL,screen,&dest);
        else SDL_BlitSurface(surface_bar_inactive,NULL,screen,&dest);

        text = TTF_RenderText_Blended(font,menu_items[i],color);
        if (text) {
            dest.x = SCREEN_WIDTH/2 - text->w/2;
            dest.y = SCREEN_HEIGHT - ((menu_size-i-1) * surface_bar->h) - 32 - offset;
            SDL_BlitSurface(text,NULL,screen,&dest);
            SDL_FreeSurface(text);
        }
    }
}

void drawInfo() {
    if (!game_over && !paused && !victory) return;

    SDL_Surface *text_info;
    char text[256] = "";
    SDL_Color color = {217,217,217};
    SDL_Rect dest;

    // statusbar background
    dest.x = 0;
    dest.y = SCREEN_HEIGHT - surface_bar->h;
    if (paused || game_over) SDL_BlitSurface(surface_bar_inactive,NULL,screen,&dest);
    else SDL_BlitSurface(surface_bar,NULL,screen,&dest);

    // statusbar text
    if (game_over) sprintf(text,"Game Over!");
    else if (paused) sprintf(text,"*Paused*");
    else if (victory) sprintf(text,"You made it across!");

    text_info = TTF_RenderText_Blended(font,text,color);
    if(text_info) {
        dest.x = 8;
        dest.y = SCREEN_HEIGHT-32;

        SDL_BlitSurface(text_info,NULL,screen,&dest);
        SDL_FreeSurface(text_info);
    }

    // menu
    if (paused || game_over || victory) drawMenu(surface_bar->h);
}

void drawTitle() {
    SDL_Rect dest;

    // title logo
    dest.x = 0;
    dest.y = SCREEN_HEIGHT/2 - surface_title->h/2 - surface_bar->h*2;
    SDL_BlitSurface(surface_title,NULL,screen,&dest);

    // menu
    drawMenu(0);
}

void drawOptions() {
    // menu
    drawMenu(0);
}

void drawPlayer() {
    if (game_over) return;

    SDL_Surface *rot = sysCreateSurface(48,48);
    if (!rot) return;

    SDL_Rect dest,src;

    dest.x = 0;
    dest.y = 0;

    src.x = 0;
    src.y = 48 * player.frame;
    src.w = src.h = 48;

    SDL_BlitSurface(surface_player,&src,rot,&dest);

    rot = rotozoomSurface(rot,player.angle,1,0);
    if (!rot) return;

    dest.x = player.x + ((48-rot->w)/2) - (int)((sin(player.angle * M_PI / 180.0)*48)/2);
    dest.y = player.y + ((48-rot->h)/2);

    SDL_BlitSurface(rot,NULL,screen,&dest);

    SDL_FreeSurface(rot);
}

