#include "metallc64.h"

void	draw_bg(_VIC_II *vic, uint32_t color) {
	SDL_SetRenderDrawColor(vic->renderer, 0xFF, 0x0, 0x0, 0x0);
	SDL_RenderClear(vic->renderer);
	SDL_RenderPresent(vic->renderer);
}

void	draw_line(_VIC_II *vic, int x0, int y0, int x1, int y1, int color) {
	int	dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int	dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int	err = dx+dy, e2;

	for(;;){
		if (x0 == x1 && y0 == y1) break;
		e2 = 2*err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
		mlx_put_pixel(vic->mlx_img, x0, y0, color << 8 | 0xFF);
	}
}

void	put_pixel(_VIC_II *vic, unsigned x, unsigned y, uint32_t color) {
	unsigned new_x = x * vic->wpdx,
	         new_y = y * vic->wpdy;
	for (unsigned n_y = new_y; n_y < new_y + vic->wpdy; n_y++)
		for (unsigned n_x = new_x; n_x < new_x + vic->wpdx; n_x++) {
			if (n_x < vic->win_width && n_y < vic->win_height) 
				mlx_put_pixel(vic->mlx_img, n_x, n_y, color);
		}
}

SDL_Window *init_window(_bus * bus, _VIC_II *vic) {
	SDL_Window *win;	
	vic->wpdx = WPDX;
	vic->wpdy = WPDY;
	vic->win_height = WHEIGHT;
	vic->win_width = WWIDTH;
	if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS))
		return False;
	win = SDL_CreateWindow("MetallC64", vic->win_width, vic->win_height,
			SDL_WINDOW_RESIZABLE|
			SDL_WINDOW_ALWAYS_ON_TOP);
	if (!win || !(vic->renderer = SDL_CreateRenderer(win, NULL))) {
		bus->clean(bus);
		free(bus);
		return FALSE;
	}
	draw_bg(vic, 0x0000FFFF);
	return mlx_ptr;
}
