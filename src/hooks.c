#include "metallc64.h"

void	close_hook(void *p) {
	(void)p;
	sig_handle(0);
}

void	resize_hook(int w, int h, void *p) {
	_ppu	*ppu = (_ppu*)p;
	ppu->win_height = h;
	ppu->win_width = w;
}

void	key_hook(mlx_key_data_t keydata, void *p) {
	if (keydata.action == MLX_RELEASE) return;
	if (keydata.key == MLX_KEY_ESCAPE)
		close_hook(p);
}

void	setup_mlx_hooks(void *p) {
	_ppu	*ppu = (_ppu*)p;

	mlx_close_hook(ppu->mlx_ptr, close_hook, ppu);
	mlx_resize_hook(ppu->mlx_ptr, resize_hook, ppu);
	mlx_key_hook(ppu->mlx_ptr, key_hook, ppu);
	mlx_loop_hook(ppu->mlx_ptr, loop_hook, ppu);
}
