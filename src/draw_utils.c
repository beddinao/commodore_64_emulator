#include "metallc64.h"

void	draw_bg(_ppu *ppu, unsigned color) {
	for (unsigned y = 0; y < ppu->win_height; y++)
		for (unsigned x = 0; x < ppu->win_width; x++)
			mlx_put_pixel(ppu->mlx_img, x, y, color << 8 | 0xFF);
			//mlx_put_pixel(ppu->mlx_img, x, y, rand() % 2 ? color : 0xFF0000 << 8|0xFF);
}

void	draw_line(_ppu *ppu, int x0, int y0, int x1, int y1, int color) {
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

void	render_screen(_ppu *ppu) {
	for (unsigned y = 0; y < P_HEIGHT; y++)
		for (unsigned x = 0; x < P_WIDTH; x++)
			mlx_put_pixel(ppu->mlx_img, x, y, ppu->screen[y * P_WIDTH + x] << 0x8 | 0xFF);
}
