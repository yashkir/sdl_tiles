#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>

#define DESIRED_DEPTH 32
#define GRID_SIZE     70
#define G_SDL_SURFACE_TYPE SDL_SWSURFACE

/* =======================================================================
 *   Structs & Typedefs
 * ======================================================================= */
struct screen_layout {
    SDL_Rect screen;
    SDL_Rect map;
    SDL_Rect message;
};
struct level {
    SDL_Rect size;
    Uint32   terrain_data[10][10];
};

/* =======================================================================
 *   Globals
 * ======================================================================= */
SDL_Surface *Screen;
Uint8       g_Depth = DESIRED_DEPTH;

struct screen_layout Layout = {
    {0,  0,   640, 480},
    {10, 10,  620, 440},
    {10, 460, 620, 10 },
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

/* =======================================================================
 *   Graphics
 * ======================================================================= */
 
/* Make a surface that contains the visual representation of "level" */
SDL_Surface *
g_make_terrain_surface(struct level* level)
{
    int i, j;
    SDL_Surface *surface;
    SDL_Rect    dst = {0, 0, GRID_SIZE, GRID_SIZE};

    surface = SDL_CreateRGBSurface(G_SDL_SURFACE_TYPE,
                                   level->size.w * GRID_SIZE,
                                   level->size.h * GRID_SIZE,
                                   g_Depth, 0, 0, 0, 0);

    for (i = 0; i < level->size.w; i++) {
        for (j = 0; j < level->size.h; j++) {
            dst.x = i * GRID_SIZE;
            dst.y = j * GRID_SIZE;
            switch (level->terrain_data[i][j]) {
                case 1:
                    SDL_FillRect(surface, &dst, 0xffff0000);
                    break;
                default:
                    SDL_FillRect(surface, &dst, 0xff550000);
                    break;
            }
        }
    }

    return surface;
}

/* Pan a view over a surface */
void
g_pan_rect(SDL_Rect *rect, SDL_Surface *surf, int x, int y)
{
    rect->x = rect->x + x;
    rect->y = rect->y + y;

    /* If we have panned too far move it back */
    if ((rect->x + rect->w) > surf->w)
        rect->x = surf->w - rect->w;
    if (rect->x < 0)
        rect->x = 0;
    if ((rect->y + rect->h) > surf->h)
        rect->y = surf->h - rect->h;
    if (rect->y < 0)
        rect->y = 0;
}

/* =======================================================================
 *   Main
 * ======================================================================= */
int
main(int argc, char *args[])
{
    SDL_Surface *terrain;
    SDL_Rect    terrain_view = {0,0,0,0};
    SDL_Event   event;

    /* Initialize */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
    atexit(SDL_Quit);

    Screen = SDL_SetVideoMode(Layout.screen.w,
                              Layout.screen.h,
                              g_Depth, SDL_SWSURFACE);

    /* Do the terrain */
    terrain = g_make_terrain_surface(&Level1);

    /* Fill the regions */
    SDL_FillRect(Screen, &Layout.screen,  0x111111ff);
    SDL_FillRect(Screen, &Layout.map,     0x000055ff);
    SDL_FillRect(Screen, &Layout.message, 0x000055ff);

    /* Limit the terrain view to what the layout allows */
    terrain_view.w = Layout.map.w;
    terrain_view.h = Layout.map.h;

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    exit(0); break;
                case SDLK_DOWN:
                    g_pan_rect(&terrain_view, terrain,  0,  20); break;
                case SDLK_UP:
                    g_pan_rect(&terrain_view, terrain,  0, -20); break;
                case SDLK_LEFT:
                    g_pan_rect(&terrain_view, terrain, -20, 0 ); break;
                case SDLK_RIGHT:
                    g_pan_rect(&terrain_view, terrain,  20, 0 ); break;
                default:
                    break;
            }
        }
        /* Redraw map */
        SDL_FillRect(Screen, &Layout.map, 0x000055ff);
        SDL_BlitSurface(terrain, &terrain_view, Screen, &Layout.map);
        SDL_Flip(Screen);
        SDL_Delay(50);
    }

    exit(0);
}
