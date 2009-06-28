#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>

#define DEPTH 32
#define GRID_SIZE 70

/* The Layout */
struct screen_layout {
    SDL_Rect screen;
    SDL_Rect map;
    SDL_Rect message;
} Layout = {
    {0,  0,   640, 480},
    {10, 10,  620, 440},
    {10, 460, 620, 10 },
};

/* Structs */
struct terrain {
    SDL_Surface *surface;
    SDL_Rect    surface_size;
    SDL_Rect    grid_size;
    SDL_Rect    view;
};

struct level {
    SDL_Rect size;
    Uint32   terrain_data[10][10];
};

struct level Level1 = {
    {0,0, 10, 10},
    {
        {0,0,0,0,0,0,0,0,0,0},
        {0,1,0,0,1,0,1,1,1,0},
        {0,1,0,0,1,0,0,1,0,0},
        {0,1,0,0,1,0,0,1,0,0},
        {0,1,1,1,1,0,0,1,0,0},
        {0,1,0,0,1,0,0,1,0,0},
        {0,1,0,0,1,0,0,1,0,0},
        {0,1,0,0,1,0,1,1,1,0},
        {0,0,0,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1},
    },
};

/* Globals */
SDL_Surface *Screen;

struct terrain *
terrain_new(struct level *level)
{
    int i, j;
    struct terrain *p;
    SDL_Rect dstrect = {0, 0, GRID_SIZE, GRID_SIZE};
    
    p = (struct terrain*)malloc(sizeof(*p));
    p->surface_size.x = 0;
    p->surface_size.y = 0;
    p->surface_size.w = GRID_SIZE *level->size.w;
    p->surface_size.h = GRID_SIZE *level->size.h;

    /* Make a surface for the terrain to go on */
    p->surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                      p->surface_size.w,
                                      p->surface_size.h,
                                      DEPTH, 0, 0, 0, 0);

    for (i = 0; i < level->size.w; i++) {
        for (j = 0; j < level->size.h; j++) {
            dstrect.x = i * GRID_SIZE;
            dstrect.y = j * GRID_SIZE;
            switch (level->terrain_data[j][i]) {
                case 1:
                    SDL_FillRect(p->surface, &dstrect, 0xffff0000);
                    break;
                default:
                    SDL_FillRect(p->surface, &dstrect, 0xff550000);
                    break;
            }
        }
    }
    return p;
}

void
terrain_view_set(struct terrain* s, int x, int y) {
    s->view.x = s->view.x + x;
    if (s->view.x < 0)
        s->view.x = 0;
    s->view.y = s->view.y + y;
    if (s->view.y < 0)
        s->view.y = 0;
}
    
/* Main */
int
main(int argc, char *args[])
{
    SDL_Surface *terrain;
    SDL_Event event;
    struct terrain *map;

    /* Initialize */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
    atexit(SDL_Quit);

    Screen = SDL_SetVideoMode(Layout.screen.w,
                              Layout.screen.h,
                              DEPTH, SDL_SWSURFACE);

    /* Do the terrain */
    map = terrain_new(&Level1);

    /* Fill the regions */
    SDL_FillRect(Screen, &Layout.screen,  0x111111ff);
    SDL_FillRect(Screen, &Layout.map,     0x000055ff);
    SDL_FillRect(Screen, &Layout.message, 0x000055ff);
    map->view.w = Layout.map.w;
    map->view.h = Layout.map.h;

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    exit(0); break;
                case SDLK_DOWN:
                    terrain_view_set(map, 0, 20); break;
                case SDLK_UP:
                    terrain_view_set(map, 0, -20); break;
                case SDLK_LEFT:
                    terrain_view_set(map, -20, 0); break;
                case SDLK_RIGHT:
                    terrain_view_set(map, 20, 0); break;
                default:
                    break;
            }
        }
        /* Redraw map */
        SDL_FillRect(Screen, &Layout.map,     0x000055ff);
        SDL_BlitSurface(map->surface, &map->view, Screen, &Layout.map);
        SDL_Flip(Screen);
        SDL_Delay(50);
    }

    exit(0);
}
