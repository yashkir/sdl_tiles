#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include "error.h"

#define DESIRED_DEPTH 32
#define GRID_SIZE     64
#define G_SDL_SURFACE_TYPE SDL_SWSURFACE
#define G_COLOR_ALPHA 0xffff00ff

/* =======================================================================
 *   Structs & Typedefs
 * ======================================================================= */
struct screen_layout {
    SDL_Rect screen;
    SDL_Rect map;
    SDL_Rect message;
    SDL_Rect sidebar;
};
struct level {
    size_t x_n, y_n;
    Uint32 **terrain;
};

/* =======================================================================
 *   Globals
 * ======================================================================= */
struct screen_layout g_Layout = {
    {0,    0, 640, 480},
    {10,  10, 540, 400},
    {10, 420, 620, 50 },
    {560, 10,  70, 400},
};

SDL_Surface *g_Screen;
Uint8        g_Depth = DESIRED_DEPTH;
const char   g_Path_unit_tiles[] = "./units.bmp";
const char   g_Path_terrain_tiles[] = "./terrain.bmp";
SDL_Surface *g_Tiles_units;
SDL_Surface *g_Tiles_terrain;
int          g_Dirty = 1;


/* =======================================================================
 *   Utilities and Matrices
 * ======================================================================= */

void *
u_make_2d_array(size_t span, int i, int j)
{
    char **m = malloc(sizeof(char *) * i);
    for (--i; i >= 0; i--) {
        m[i] = malloc(span * j);
    }

    return m;
}

void
u_array_test(void)
{
    Uint32 **p;
    int i, j;
    p = u_make_2d_array(sizeof(Uint32), 5, 7);

    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            p[i][j] = 55;
            printf("%d:%d %d ", i, j, p[i][j]);
        }
    }
    printf("\n");
}

/* =======================================================================
 *   Graphics
 * ======================================================================= */

/* Create a g_Screen surface */
void
g_init_first(void)
{
    g_Screen = SDL_SetVideoMode(g_Layout.screen.w,
                                g_Layout.screen.h,
                                g_Depth, SDL_SWSURFACE);
    
    if (!g_Screen)
        error(ERR_FATAL, "Cannot create window: %s\n", SDL_GetError());
}

/* Loads all the tilesets into memory */
void
g_init_tiles(void)
{
    g_Tiles_units = SDL_LoadBMP(g_Path_unit_tiles);
    if (!g_Tiles_units)
        error(ERR_FATAL, "Cannot load file: %s\n", SDL_GetError());

    g_Tiles_terrain = SDL_LoadBMP(g_Path_terrain_tiles);
    if (!g_Tiles_terrain)
        error(ERR_FATAL, "Cannot load file: %s\n", SDL_GetError());
}

/* Calculate the rectangle of a tile in a tileset */
SDL_Rect
g_whereis_tile(int x, int y, int grid_size)
{
    SDL_Rect where = {0, 0, grid_size, grid_size};

    where.x = x * grid_size;
    where.y = y * grid_size;

    return where;
}
 
/* Make a surface that has appropriate dimensions for the "level" */
SDL_Surface *
g_make_terrain_surface(struct level* level)
{
    SDL_Surface *surface;

    surface = SDL_CreateRGBSurface(G_SDL_SURFACE_TYPE,
                                   level->x_n * GRID_SIZE,
                                   level->y_n * GRID_SIZE,
                                   g_Depth, 0, 0, 0, 0);

    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, G_COLOR_ALPHA);
    SDL_FillRect(surface, NULL, G_COLOR_ALPHA);

    if (!surface)
        error(ERR_FATAL, "Surface Error: %s\n", SDL_GetError());

    return surface;
}

/* Create a surface containing one tile from a tileset */
SDL_Surface *
g_make_tile(SDL_Surface *tileset, int x, int y, int grid_size)
{
    SDL_Surface *tile;
    SDL_Rect    selected_tile = {0, 0, grid_size, grid_size};

    tile = SDL_CreateRGBSurface(G_SDL_SURFACE_TYPE,
                                grid_size, grid_size,
                                g_Depth, 0, 0, 0, 0);

    if (!tile)
        error(ERR_FATAL, "Cannot create surface: %s\n", SDL_GetError);

    selected_tile.x = x * grid_size;
    selected_tile.y = y * grid_size;
    SDL_FillRect(tile, NULL, 0xff00ff00);
    SDL_BlitSurface(tileset, &selected_tile, tile, NULL);
    
    return tile;
}

/* Render the terrain of the level onto the surface */
void
g_render_terrain(SDL_Surface *surface, struct level* level)
{
    int i, j;
    SDL_Rect    dst = {0, 0, GRID_SIZE, GRID_SIZE};
    SDL_Rect    src;

    for (i = 0; i < level->x_n; i++) {
        for (j = 0; j < level->y_n; j++) {
            dst.x = i * GRID_SIZE;
            dst.y = j * GRID_SIZE;
            switch (level->terrain[i][j]) {
                case 0:
                    src = g_whereis_tile(0, 0, GRID_SIZE);
                    SDL_BlitSurface(g_Tiles_terrain, &src, surface, &dst);
                    break;
                case 1:
                    src = g_whereis_tile(1, 0, GRID_SIZE);
                    SDL_BlitSurface(g_Tiles_terrain, &src, surface, &dst);
                    break;
                case 2:
                    src = g_whereis_tile(0, 0, GRID_SIZE);
                    SDL_BlitSurface(g_Tiles_terrain, &src, surface, &dst);
                    break;
                default:
                    SDL_FillRect(surface, &dst, 0xff550000);
                    break;
            }
        }
    }
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

    g_Dirty = 1;
}

/* Draw a unit with grid offset onto surface */
void
g_draw_unit(SDL_Surface *unit, SDL_Surface *dst, int x, int y)
{
    SDL_Rect dstrect = {0, 0, GRID_SIZE, GRID_SIZE};

    dstrect.x = x * GRID_SIZE;
    dstrect.y = y * GRID_SIZE;

    SDL_BlitSurface(unit, NULL, dst, &dstrect);
}

/* TODO: better interface */
void
g_coord_from_click(SDL_Rect *view, int x, int y, int *yset, int *xset)
{
    *xset = (x + view->x) / GRID_SIZE;
    *yset = (y + view->y) / GRID_SIZE;
}

void
g_render_screen(void)
{

}

/* =======================================================================
 *   Timing
 * ======================================================================= */
static Uint32 t_Milliseconds_per_frame = 30;

Uint32
t_time_left_in_frame(void)
{
    static Uint32 next = 0;
    Uint32 now = SDL_GetTicks();

    if (now > next) {
        next = now + t_Milliseconds_per_frame;
        return 0;
    }
    else {
        return next - now;
    }
}

/* =======================================================================
 *   Game and Logic (control)
 * ======================================================================= */

/* Handle SDL init and anything that needs to be done _first_ */
void
c_init_first(void)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
    atexit(SDL_Quit);
}

/* Start a simple tech demo */
void
c_start_demo(void)
{

}

/* =======================================================================
 *   Files and Configuration
 * ======================================================================= */
struct level *
f_load_level(char *filename)
{
    FILE *fp;
    struct level *level;
    int x, y;
    int i, j;

    if (!(fp = fopen(filename, "r")))
        error(ERR_FATAL, "Cannot open file: %s\n", filename);
    
    fscanf(fp, "%d,%d", &x, &y);

    level          = calloc(1, sizeof(level));
    level->terrain = (Uint32**)u_make_2d_array(sizeof(Uint32), x, y);
    level->x_n = x;
    level->y_n = y;

    /* TODO: clean up dimensions and rotation, maybe */
    for (j = 0; j < y; j++) {
        for (i = 0; i < x; i++) {
            fscanf(fp, "%d", &level->terrain[i][j]);
        }
    }

    return level;
}

/* =======================================================================
 *   User Interface
 * ======================================================================= */
void
u_redraw(void)
{

}

/* =======================================================================
 *   Main
 * ======================================================================= */
int
main(int argc, char *args[])
{
    SDL_Surface *terrain, *unit_layer, *unit, *unit2;
    SDL_Rect    terrain_view = {0,0,0,0};
    SDL_Event   event;
    struct level *level;
    int         temp = 4;

    /* Initialize */
    c_init_first();

    g_Screen = SDL_SetVideoMode(g_Layout.screen.w,
                                g_Layout.screen.h,
                                g_Depth, SDL_SWSURFACE);

    level = f_load_level("./level1.csv");

    /* Create surfaces */
    g_init_tiles();
    terrain     = g_make_terrain_surface(level);
    unit_layer  = g_make_terrain_surface(level);
    unit        = g_make_tile(g_Tiles_units, 0, 0, GRID_SIZE);
    unit2       = g_make_tile(g_Tiles_units, 1, 0, GRID_SIZE);
    g_render_terrain(terrain, level);

    /* Fill the regions */
    SDL_FillRect(g_Screen, &g_Layout.screen,  0x111111ff);
    SDL_FillRect(g_Screen, &g_Layout.map,     0x000055ff);
    SDL_FillRect(g_Screen, &g_Layout.message, 0x000055ff);
    SDL_FillRect(g_Screen, &g_Layout.sidebar, 0x000055ff);

    /* Limit the terrain view to what the layout allows */
    terrain_view.w = g_Layout.map.w;
    terrain_view.h = g_Layout.map.h;

    SDL_FillRect(g_Screen, &g_Layout.map, 0x000055ff);

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            switch (event.button.button) {
                case 1:
                    printf("%d, %d\n", event.button.x, event.button.y);
                    {
                        int a, b;
                        g_coord_from_click(&terrain_view, event.button.x, event.button.y,
                                &a, &b);
                        printf("%d, %d\n", a, b);
                    }
                    break;
                default:
                    break;
            }
        }
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
                case SDLK_a:
                    temp += 1;
                    g_Dirty = 1;
                    break;
                default:
                    break;
            }
        }
        if (g_Dirty) {
            /* Redraw unit layer TODO: dirty flag */
            SDL_FillRect(unit_layer, NULL, G_COLOR_ALPHA);
            g_draw_unit(unit,  unit_layer, temp, temp);
            g_draw_unit(unit2, unit_layer, 5, 5);

            /* Blit the terrain and then the unit layer on top */
            SDL_BlitSurface(terrain,    &terrain_view, g_Screen, &g_Layout.map);
            SDL_BlitSurface(unit_layer, &terrain_view, g_Screen, &g_Layout.map);
            SDL_Flip(g_Screen);

            g_Dirty = 0;
        }

        SDL_Delay(t_time_left_in_frame());
    }

    exit(0);
}
