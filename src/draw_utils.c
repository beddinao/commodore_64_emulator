#include "metallc64.h"

void	draw_bg(_VIC_II *ppu, uint32_t color) {
	for (unsigned y = 0; y < ppu->win_height; y++)
		for (unsigned x = 0; x < ppu->win_width; x++)
			mlx_put_pixel(ppu->mlx_img, x, y, color);
}

void	draw_line(_VIC_II *ppu, int x0, int y0, int x1, int y1, int color) {
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
		mlx_put_pixel(ppu->mlx_img, x0, y0, color << 8 | 0xFF);
	}
}

void	put_pixel(mlx_image_t *mlx_img, unsigned x, unsigned y, uint32_t color) {
	unsigned new_x = x * WPDX,
	         new_y = y * WPDY;
	for (unsigned n_y = new_y; n_y < new_y + WPDY; n_y++)
		for (unsigned n_x = new_x; n_x < new_x + WPDX; n_x++) {
			if (n_x < WWIDTH && n_y < WHEIGHT)
				mlx_put_pixel(mlx_img, n_x, n_y, color);
		}
}
