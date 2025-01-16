#include "metallc64.h"

void	close_hook(void *p) {
	(void)p;
	sig_handle(0);
}

void	resize_hook(int w, int h, void *p) {
	_VIC_II	*vic = (_VIC_II*)p;
	vic->win_height = h;
	vic->win_width = w;
	vic->wpdy = h / GHEIGHT;
	vic->wpdx = w / GWIDTH;
}

void	set_key(_keymap *keys, uint8_t row, uint8_t col, action_t act) {
	keys->matrix[row] = 0xFF;
	if (act) keys->matrix[row] &= ~(1 << col);
}

void	key_hook(mlx_key_data_t keydata, void *p) {
	_VIC_II	*vic = (_VIC_II*)p;
	_bus	*bus = vic->bus;
	_keymap	*keys = ((_CIA*)bus->cia1)->keys;
	
	switch (keydata.key) {
		case MLX_KEY_KP_0: keydata.key = MLX_KEY_0; break;
		case MLX_KEY_KP_1: keydata.key = MLX_KEY_1; break;
		case MLX_KEY_KP_2: keydata.key = MLX_KEY_2; break;
		case MLX_KEY_KP_3: keydata.key = MLX_KEY_3; break;
		case MLX_KEY_KP_4: keydata.key = MLX_KEY_4; break;
		case MLX_KEY_KP_5: keydata.key = MLX_KEY_5; break;
		case MLX_KEY_KP_6: keydata.key = MLX_KEY_6; break;
		case MLX_KEY_KP_7: keydata.key = MLX_KEY_7; break;
		case MLX_KEY_KP_8: keydata.key = MLX_KEY_8; break;
		case MLX_KEY_KP_9: keydata.key = MLX_KEY_9; break;
		case MLX_KEY_KP_ENTER: keydata.key = MLX_KEY_ENTER; break;
		case MLX_KEY_KP_EQUAL: keydata.key = MLX_KEY_EQUAL; break;
		case MLX_KEY_LEFT_CONTROL: keydata.key = MLX_KEY_RIGHT_CONTROL; break;
		case MLX_KEY_DELETE: keydata.key = MLX_KEY_BACKSPACE; break;
		case MLX_KEY_LEFT_SUPER: keydata.key = MLX_KEY_RIGHT_SUPER; break;
		default:	break;
	}

	switch (keydata.key) {
		/* CTRL */
		case MLX_KEY_ESCAPE: set_key(keys, 7, 7, keydata.action); break;
		case MLX_KEY_RIGHT_SUPER: set_key(keys, 7, 5, keydata.action); break;
		case MLX_KEY_BACKSPACE: set_key(keys, 0, 0, keydata.action); break;
		case MLX_KEY_ENTER: set_key(keys, 0, 1, keydata.action); break;
		case MLX_KEY_F1: set_key(keys, 0, 4, keydata.action); break;
		case MLX_KEY_F2: set_key(keys, 0, 5, keydata.action); break;
		case MLX_KEY_F5: set_key(keys, 0, 6, keydata.action); break;
		case MLX_KEY_F7: set_key(keys, 0, 3, keydata.action); break;
		case MLX_KEY_LEFT_SHIFT: set_key(keys, 1, 7, keydata.action); break;
		case MLX_KEY_RIGHT_SHIFT: set_key(keys, 6, 4, keydata.action); break;
		case MLX_KEY_RIGHT_CONTROL: set_key(keys, 7, 2, keydata.action); break;
		/* CURSOR ARROWS */
		case MLX_KEY_RIGHT: set_key(keys, 0, 2, keydata.action); break;
		case MLX_KEY_DOWN: set_key(keys, 0, 7, keydata.action); break;
		case MLX_KEY_UP:
			         set_key(keys, 1, 7, keydata.action);
			         set_key(keys, 0, 7, keydata.action);
			         break;
		case MLX_KEY_LEFT:
			         set_key(keys, 1, 7, keydata.action);
			         set_key(keys, 0, 2, keydata.action);
			         break;
		/* NUMS */
		case MLX_KEY_1: set_key(keys, 7, 0, keydata.action); break;
		case MLX_KEY_2: set_key(keys, 7, 3, keydata.action); break;
		case MLX_KEY_3: set_key(keys, 1, 0, keydata.action); break;
		case MLX_KEY_4: set_key(keys, 1, 3, keydata.action); break;
		case MLX_KEY_5: set_key(keys, 2, 0, keydata.action); break;
		case MLX_KEY_6: set_key(keys, 2, 3, keydata.action); break;
		case MLX_KEY_7: set_key(keys, 3, 0, keydata.action); break;
		case MLX_KEY_8: set_key(keys, 3, 3, keydata.action); break;
		case MLX_KEY_9: set_key(keys, 4, 0, keydata.action); break;
		case MLX_KEY_0: set_key(keys, 4, 3, keydata.action); break;
		/* ALPHA */
		case MLX_KEY_Q: set_key(keys, 7, 6, keydata.action); break;
		case MLX_KEY_W: set_key(keys, 1, 1, keydata.action); break;
		case MLX_KEY_E: set_key(keys, 1, 6, keydata.action); break;
		case MLX_KEY_R: set_key(keys, 2, 1, keydata.action); break;
		case MLX_KEY_T: set_key(keys, 2, 6, keydata.action); break;
		case MLX_KEY_Y: set_key(keys, 3, 1, keydata.action); break;
		case MLX_KEY_U: set_key(keys, 3, 6, keydata.action); break;
		case MLX_KEY_I: set_key(keys, 4, 1, keydata.action); break;
		case MLX_KEY_O: set_key(keys, 4, 6, keydata.action); break;
		case MLX_KEY_P: set_key(keys, 5, 1, keydata.action); break;
		case MLX_KEY_A: set_key(keys, 1, 2, keydata.action); break;
		case MLX_KEY_S: set_key(keys, 1, 5, keydata.action); break;
		case MLX_KEY_D: set_key(keys, 2, 2, keydata.action); break;
		case MLX_KEY_F: set_key(keys, 2, 5, keydata.action); break;
		case MLX_KEY_G: set_key(keys, 3, 2, keydata.action); break;
		case MLX_KEY_H: set_key(keys, 3, 5, keydata.action); break;
		case MLX_KEY_J: set_key(keys, 4, 2, keydata.action); break;
		case MLX_KEY_K: set_key(keys, 4, 5, keydata.action); break;
		case MLX_KEY_L: set_key(keys, 5, 2, keydata.action); break;
		case MLX_KEY_Z: set_key(keys, 1, 4, keydata.action); break;
		case MLX_KEY_X: set_key(keys, 2, 7, keydata.action); break;
		case MLX_KEY_C: set_key(keys, 2, 4, keydata.action); break;
		case MLX_KEY_V: set_key(keys, 3, 7, keydata.action); break;
		case MLX_KEY_B: set_key(keys, 3, 4, keydata.action); break;
		case MLX_KEY_N: set_key(keys, 4, 7, keydata.action); break;
		case MLX_KEY_M: set_key(keys, 4, 4, keydata.action); break;
		/* ARITHMATICS */
		case MLX_KEY_KP_ADD: set_key(keys, 5, 0, keydata.action); break;
		case MLX_KEY_KP_SUBTRACT: set_key(keys, 5, 3, keydata.action); break;
		case MLX_KEY_KP_MULTIPLY: set_key(keys, 6, 1, keydata.action); break;
		case MLX_KEY_EQUAL: set_key(keys, 6, 5, keydata.action); break;
		/* OTHER */
		case MLX_KEY_APOSTROPHE: set_key(keys, 5, 5, keydata.action); break;
		case MLX_KEY_PERIOD: set_key(keys, 5, 4, keydata.action); break;
		case MLX_KEY_SLASH: set_key(keys, 6, 7, keydata.action); break;
		case MLX_KEY_COMMA: set_key(keys, 5, 7, keydata.action); break;
		case MLX_KEY_SPACE: set_key(keys, 7, 4, keydata.action); break;
		case MLX_KEY_SEMICOLON: set_key(keys, 6, 2, keydata.action); break;
		default:	break;
	}
}

void	setup_mlx_hooks(void *p) {
	_VIC_II *vic = (_VIC_II*)p;
	mlx_close_hook(vic->mlx_ptr, close_hook, vic);
	mlx_resize_hook(vic->mlx_ptr, resize_hook, vic);
	mlx_key_hook(vic->mlx_ptr, key_hook, vic);
	mlx_loop_hook(vic->mlx_ptr, loop_hook, vic);
}
