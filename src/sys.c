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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys/stat.h"
#include "sys/types.h"

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"

#include "sys.h"

SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
SDL_Surface* surface_bar = NULL;
SDL_Surface* surface_bar_inactive = NULL;
SDL_Surface* surface_background = NULL;
SDL_Surface* surface_title = NULL;
SDL_Surface* surface_player = NULL;
Mix_Music* music = NULL;
Mix_Chunk* sound_menu = NULL;
SDL_Joystick* joy = NULL;

bool title_screen = true;
int options_screen = -1;
bool victory = false;
bool game_over = false;
bool trigger_game_over = false;
bool paused = false;
bool quit = false;

int action_cooldown = 0;
bool action_left = false;
bool action_right = false;
bool action_up = false;
bool action_down = false;
bool action_accept = false;
bool action_pause = false;

char* config_folder = NULL;
int option_joystick = -1;
int option_sound = 8;
int option_music = 8;
int option_fullscreen = 0;

bool sysInit() {
    if(SDL_Init(SDL_INIT_EVERYTHING) == -1) return false;
    
    screen = SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,SDL_HWSURFACE);

    if(screen == NULL) return false;
    
    if(TTF_Init() == -1) return false;

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1 ) return false;
    
    SDL_WM_SetCaption("Woods",NULL);

    sysConfigSetFolder();
    sysConfigLoad();

    return true;
}

bool sysLoadFiles() {
    // font
    font = TTF_OpenFont(PKGDATADIR "/fonts/Alegreya-Regular.ttf",20);
    if(!font) return false;
    else TTF_SetFontHinting(font, TTF_HINTING_LIGHT);

    // graphics
    surface_bar = IMG_Load(PKGDATADIR "/graphics/bar.png");
    if (!surface_bar) return false;
    else {
        SDL_Surface *cleanup = surface_bar;
        surface_bar = SDL_DisplayFormatAlpha(surface_bar);
        SDL_FreeSurface(cleanup);
    }

    surface_bar_inactive = IMG_Load(PKGDATADIR "/graphics/bar_inactive.png");
    if (!surface_bar_inactive) return false;
    else {
        SDL_Surface *cleanup = surface_bar_inactive;
        surface_bar_inactive = SDL_DisplayFormatAlpha(surface_bar_inactive);
        SDL_FreeSurface(cleanup);
    }

    surface_background = IMG_Load(PKGDATADIR "/graphics/background.png");
    if (!surface_background) return false;
    else {
        SDL_Surface *cleanup = surface_background;
        surface_background = SDL_DisplayFormat(surface_background);
        SDL_FreeSurface(cleanup);
    }

    surface_title = IMG_Load(PKGDATADIR "/graphics/title.png");
    if (!surface_title) return false;
    else {
        SDL_Surface *cleanup = surface_title;
        surface_title = SDL_DisplayFormatAlpha(surface_title);
        SDL_FreeSurface(cleanup);
    }

    surface_player = IMG_Load(PKGDATADIR "/graphics/player.png");
    if (!surface_player) return false;
    else {
        SDL_Surface *cleanup = surface_player;
        surface_player = SDL_DisplayFormatAlpha(surface_player);
        SDL_FreeSurface(cleanup);
    }

    // background music
    music = Mix_LoadMUS(PKGDATADIR "/sounds/music.ogg");
    if (!music) return false;

    // sound effects
    sound_menu = Mix_LoadWAV(PKGDATADIR "/sounds/menu.ogg");
    if (!sound_menu) return false;

    return true;
}

void sysCleanup() {
    sysConfigSave();
    if (config_folder) free(config_folder);

    Mix_HaltMusic();

    TTF_CloseFont(font);
    SDL_FreeSurface(surface_bar);
    SDL_FreeSurface(surface_bar_inactive);
    SDL_FreeSurface(surface_background);
    SDL_FreeSurface(surface_player);
    Mix_FreeMusic(music);
    Mix_FreeChunk(sound_menu);
    SDL_Quit();
}

void sysInput() {
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_LEFT)
                action_left = true;
            if(event.key.keysym.sym == SDLK_RIGHT)
                action_right = true;
            if(event.key.keysym.sym == SDLK_UP)
                action_up = true;
            if(event.key.keysym.sym == SDLK_DOWN)
                action_down = true;
            if(event.key.keysym.sym == 'z')
                action_accept = true;
            if(event.key.keysym.sym == SDLK_ESCAPE)
                action_pause = true;
        }

        else if(event.type == SDL_KEYUP) {
            if(event.key.keysym.sym == SDLK_LEFT)
                action_left = false;
            if(event.key.keysym.sym == SDLK_RIGHT)
                action_right = false;
            if(event.key.keysym.sym == SDLK_UP)
                action_up = false;
            if(event.key.keysym.sym == SDLK_DOWN)
                action_down = false;
            if(event.key.keysym.sym == 'z')
                action_accept = false;
            if(event.key.keysym.sym == SDLK_ESCAPE)
                action_pause = false;
        }

        else if(event.type == SDL_JOYBUTTONDOWN) {
            if(event.jbutton.which == 0) {
                if (event.jbutton.button == 0)
                    action_accept = true;
                if (event.jbutton.button == 9)
                    action_pause = true;
            }
        }

        else if(event.type == SDL_JOYBUTTONUP) {
            if(event.jbutton.which == 0) {
                if (event.jbutton.button == 0)
                    action_accept = false;
                if (event.jbutton.button == 9)
                    action_pause = false;
            }
        }

        else if(event.type == SDL_QUIT) {
            quit = true;
        }
    }

    if (joy) {
        int joy_x = SDL_JoystickGetAxis(joy, 0);
        int joy_y = SDL_JoystickGetAxis(joy, 1);

        // x axis
        if (joy_x < -JOY_DEADZONE) {
            action_left = true;
            action_right = false;
        } else if (joy_x > JOY_DEADZONE) {
            action_left = false;
            action_right = true;
        } else {
            action_left = false;
            action_right = false;
        }

        // y axis
        if (joy_y < -JOY_DEADZONE) {
            action_up = true;
            action_down = false;
        } else if (joy_y > JOY_DEADZONE) {
            action_up = false;
            action_down = true;
        } else {
            action_up = false;
            action_down = false;
        }
    }
}

void sysConfigSetFolder() {
    char *home = malloc(strlen(getenv("HOME"))+1);
    strcpy(home,getenv("HOME"));

    config_folder = malloc(strlen(home)+strlen("/.woods")+1);
    sprintf(config_folder,"%s/.woods",home);

    free(home);
}

void sysConfigLoad() {
    FILE *config_file;
    char buffer[BUFSIZ];
    char *key;
    char *temp;

    mkdir(config_folder, S_IRWXU | S_IRWXG | S_IRWXO);
    char *config_path = malloc(strlen(config_folder)+strlen("/config")+1);

    if (config_path) {
        sprintf(config_path,"%s/config",config_folder);
        config_file = fopen(config_path,"r+");

        if (config_file) {
            while (fgets(buffer,BUFSIZ,config_file)) {
                temp = buffer;
                if (temp[0] == '#') continue;
                key = strtok(temp,"=");
                if (strcmp(key,"joystick") == 0) option_joystick = atoi(strtok(NULL,"\n"));
                if (strcmp(key,"sound") == 0) option_sound = atoi(strtok(NULL,"\n"));
                if (strcmp(key,"music") == 0) option_music = atoi(strtok(NULL,"\n"));
                if (strcmp(key,"fullscreen") == 0) option_fullscreen = atoi(strtok(NULL,"\n"));
            }
            fclose(config_file);
            sysConfigApply();
        } else {
            printf ("Error: Couldn't load config file. Creating new config...\n");
            sysConfigSave();
        }

        free(config_path);
    }
}

void sysConfigSave() {
    FILE *config_file;

    mkdir(config_folder, S_IRWXU | S_IRWXG | S_IRWXO);
    char *config_path = malloc(strlen(config_folder)+strlen("/config")+1);

    if (config_path) {
        sprintf(config_path,"%s/config",config_folder);
        config_file = fopen(config_path,"w+");

        if (config_file) {
            fprintf(config_file,"joystick=%d\n",option_joystick);
            fprintf(config_file,"sound=%d\n",option_sound);
            fprintf(config_file,"music=%d\n",option_music);
            fprintf(config_file,"fullscreen=%d\n",option_fullscreen);
            fclose(config_file);
        } else printf("Error: Couldn't write to config file.\n");

        free(config_path);
    }

    sysConfigApply();
}

void sysConfigApply() {
    if (joy) SDL_JoystickClose(joy);
    if (SDL_NumJoysticks() > 0 && option_joystick > -1) joy = SDL_JoystickOpen(option_joystick);
    else {
        option_joystick = -1;
        joy = NULL;
    }

    Mix_Volume(-1,option_sound*16);
    Mix_VolumeMusic(option_music*16);

    if (option_fullscreen == 1) {
        screen = SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,SDL_HWSURFACE|SDL_FULLSCREEN);
    }
    if (!screen || option_fullscreen != 1) {
        screen = SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,SDL_HWSURFACE);
    }
}

SDL_Surface* sysCreateSurface(int width, int height) {

	SDL_Surface *surface;
	Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, rmask, gmask, bmask, amask);

	if(surface == NULL) return NULL;

	SDL_Surface *cleanup = surface;
	surface = SDL_DisplayFormat(surface);
	SDL_FreeSurface(cleanup);

	return surface;
}

