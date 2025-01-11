#include "metallc64.h"

void	draw_bg(_VIC_II *vic, uint32_t color) {
	for (unsigned y = 0; y < vic->win_height; y++)
		for (unsigned x = 0; x < vic->win_width; x++)
			mlx_put_pixel(vic->mlx_img, x, y, color);
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

mlx_t	*init_window(_bus * bus, _VIC_II *vic) {
	mlx_t *mlx_ptr;
	vic->wpdx = WPDX;
	vic->wpdy = WPDY;
	vic->win_height = WHEIGHT;
	vic->win_width = WWIDTH;
	mlx_ptr = mlx_init(vic->win_width, vic->win_height, "MetallC64", true);
	if (!mlx_ptr || !(vic->mlx_img = mlx_new_image(mlx_ptr, vic->win_width, vic->win_height))) {
		bus->clean(bus);
		free(bus);
		return FALSE;
	}
	draw_bg(vic, 0x0000FFFF);
	return mlx_ptr;
}
