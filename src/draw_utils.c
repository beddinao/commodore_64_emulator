#include <c64_emu.h> 

void	draw_bg(_VIC_II *vic, uint32_t color) {
	SDL_SetRenderDrawColor(vic->renderer,
		(color >> 24) & 0xFF,
		(color >> 16) & 0xFF,
		(color >> 8) & 0xFF,
		0xFF);
	SDL_RenderClear(vic->renderer);
}

void	put_pixel(_VIC_II *vic, unsigned x, unsigned y, uint32_t color) {
	unsigned new_x = x * vic->wpdx,
	         new_y = y * vic->wpdy,
	         count = vic->wpdx * vic->wpdy * 2,
	         pIndex = 0;

	SDL_SetRenderDrawColor(vic->renderer,
		(color >> 24) & 0xFF,
		(color >> 16) & 0xFF,
		(color >> 8) & 0xFF,
		0xFF);

	SDL_FPoint points[count];

	for (unsigned n_y = new_y; n_y < new_y + vic->wpdy && pIndex < count; n_y++)
		for (unsigned n_x = new_x; n_x < new_x + vic->wpdx && pIndex < count; n_x++) {
			points[pIndex].x = n_x;
			points[pIndex++].y = n_y;
		}

	SDL_RenderPoints(vic->renderer, points, pIndex);
}

void	put_raster(_VIC_II *vic, unsigned raster, unsigned x0, unsigned x1, uint32_t color) {
	SDL_SetRenderDrawColor(vic->renderer,
		(color >> 24) & 0xFF,
		(color >> 16) & 0xFF,
		(color >> 8) & 0xFF,
		0xFF);

	SDL_RenderLine(vic->renderer, x0, raster, x1, raster);
}

SDL_Window *init_window(_bus * bus, _VIC_II *vic) {
	SDL_Window *win = NULL;
	vic->wpdx = WPDX;
	vic->wpdy = WPDY;
	vic->win_height = WHEIGHT;
	vic->win_width = WWIDTH;
	if (!SDL_Init(SDL_INIT_EVENTS))
		return FALSE;
	win = SDL_CreateWindow("c64-emu", vic->win_width, vic->win_height,
			SDL_WINDOW_RESIZABLE/*|
			SDL_WINDOW_ALWAYS_ON_TOP*/);
	if (!win || !(vic->renderer = SDL_CreateRenderer(win, NULL))) {
		if (win) SDL_DestroyWindow(win);
		bus->clean(bus);
		free(bus);
		return FALSE;
	}
	SDL_SetWindowMinimumSize(win, MWIDTH, MHEIGHT);
	draw_bg(vic, 0x0000FFFF);
	return win;
}
