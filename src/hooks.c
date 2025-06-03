#include <c64_emu.h> 

void	window_event_handle(_bus* bus) {
	int new_w;
	//pthread_mutex_lock(&bus->t_data->prg_mutex);
	_VIC_II *vic = (_VIC_II*)bus->vic;
	if (!SDL_GetWindowSize(vic->win, &new_w, NULL))
		return;
	float new_wpd = (float)new_w / (float)GWIDTH;
	if (new_wpd != vic->wpdx) {
		vic->win_width = new_w;
		vic->wpdx = new_wpd;
		vic->wpdy = vic->wpdx;
		vic->win_height = vic->win_width * HTOW;
	}
	//pthread_mutex_unlock(&bus->t_data->prg_mutex);
}

void	set_key(_keymap *keys, uint8_t row, uint8_t col, bool act) {
	keys->matrix[row] = 0xFF;
	if (act) keys->matrix[row] &= ~(1 << col);
}

void	key_event_handle(_bus* bus, SDL_Event *event, bool action) {
	_keymap *keys = ((_CIA*)bus->cia1)->keys;
	SDL_Keycode key = event->key.key;
	
	switch (key) {
		case SDLK_KP_0: key = SDLK_0; break;
		case SDLK_KP_1: key = SDLK_1; break;
		case SDLK_KP_2: key = SDLK_2; break;
		case SDLK_KP_3: key = SDLK_3; break;
		case SDLK_KP_4: key = SDLK_4; break;
		case SDLK_KP_5: key = SDLK_5; break;
		case SDLK_KP_6: key = SDLK_6; break;
		case SDLK_KP_7: key = SDLK_7; break;
		case SDLK_KP_8: key = SDLK_8; break;
		case SDLK_KP_9: key = SDLK_9; break;
		case SDLK_KP_PLUS: key = SDLK_PLUS; break;
		case SDLK_KP_MINUS: key = SDLK_MINUS; break;
		case SDLK_ASTERISK: key = SDLK_KP_MULTIPLY; break;
		case SDLK_KP_EQUALS: key = SDLK_EQUALS; break;
		case SDLK_LCTRL: key = SDLK_RCTRL; break;
		case SDLK_DELETE: key = SDLK_BACKSPACE; break;
		case SDLK_KP_BACKSPACE: key = SDLK_BACKSPACE; break;
		case SDLK_KP_PERIOD: key = SDLK_PERIOD; break;
		case SDLK_KP_COMMA: key = SDLK_COMMA; break;
		case SDLK_KP_SPACE: key = SDLK_SPACE; break;
		case SDLK_KP_ENTER: key = SDLK_RETURN; break;
		default:	break;
	}

	switch (key) {
		/* CTRL */
		case SDLK_ESCAPE: set_key(keys, 7, 7, action); break;
		case SDLK_BACKSPACE: set_key(keys, 0, 0, action); break;
		case SDLK_RETURN: set_key(keys, 0, 1, action); break;
		case SDLK_F1: set_key(keys, 0, 4, action); break;
		case SDLK_F2: set_key(keys, 0, 5, action); break;
		case SDLK_F5: set_key(keys, 0, 6, action); break;
		case SDLK_F7: set_key(keys, 0, 3, action); break;
		case SDLK_LSHIFT: set_key(keys, 1, 7, action); break;
		case SDLK_RSHIFT: set_key(keys, 6, 4, action); break;
		case SDLK_RCTRL: set_key(keys, 7, 2, action); break;
		/* CURSOR ARROWS */
		case SDLK_RIGHT: set_key(keys, 0, 2, action); break;
		case SDLK_DOWN: set_key(keys, 0, 7, action); break;
		case SDLK_UP:
			         set_key(keys, 1, 7, action);
			         set_key(keys, 0, 7, action);
			         break;
		case SDLK_LEFT:
			         set_key(keys, 1, 7, action);
			         set_key(keys, 0, 2, action);
			         break;
		/* NUMS */
		case SDLK_1: set_key(keys, 7, 0, action); break;
		case SDLK_2: set_key(keys, 7, 3, action); break;
		case SDLK_3: set_key(keys, 1, 0, action); break;
		case SDLK_4: set_key(keys, 1, 3, action); break;
		case SDLK_5: set_key(keys, 2, 0, action); break;
		case SDLK_6: set_key(keys, 2, 3, action); break;
		case SDLK_7: set_key(keys, 3, 0, action); break;
		case SDLK_8: set_key(keys, 3, 3, action); break;
		case SDLK_9: set_key(keys, 4, 0, action); break;
		case SDLK_0: set_key(keys, 4, 3, action); break;
		/* ALPHA */
		case SDLK_Q: set_key(keys, 7, 6, action); break;
		case SDLK_W: set_key(keys, 1, 1, action); break;
		case SDLK_E: set_key(keys, 1, 6, action); break;
		case SDLK_R: set_key(keys, 2, 1, action); break;
		case SDLK_T: set_key(keys, 2, 6, action); break;
		case SDLK_Y: set_key(keys, 3, 1, action); break;
		case SDLK_U: set_key(keys, 3, 6, action); break;
		case SDLK_I: set_key(keys, 4, 1, action); break;
		case SDLK_O: set_key(keys, 4, 6, action); break;
		case SDLK_P: set_key(keys, 5, 1, action); break;
		case SDLK_A: set_key(keys, 1, 2, action); break;
		case SDLK_S: set_key(keys, 1, 5, action); break;
		case SDLK_D: set_key(keys, 2, 2, action); break;
		case SDLK_F: set_key(keys, 2, 5, action); break;
		case SDLK_G: set_key(keys, 3, 2, action); break;
		case SDLK_H: set_key(keys, 3, 5, action); break;
		case SDLK_J: set_key(keys, 4, 2, action); break;
		case SDLK_K: set_key(keys, 4, 5, action); break;
		case SDLK_L: set_key(keys, 5, 2, action); break;
		case SDLK_Z: set_key(keys, 1, 4, action); break;
		case SDLK_X: set_key(keys, 2, 7, action); break;
		case SDLK_C: set_key(keys, 2, 4, action); break;
		case SDLK_V: set_key(keys, 3, 7, action); break;
		case SDLK_B: set_key(keys, 3, 4, action); break;
		case SDLK_N: set_key(keys, 4, 7, action); break;
		case SDLK_M: set_key(keys, 4, 4, action); break;
		/* ARITHMATICS */
		case SDLK_PLUS: set_key(keys, 5, 0, action); break;
		case SDLK_MINUS: set_key(keys, 5, 3, action); break;
		case SDLK_KP_MULTIPLY: set_key(keys, 6, 1, action); break;
		case SDLK_EQUALS: set_key(keys, 6, 5, action); break;
		/* OTHER */
		case SDLK_APOSTROPHE: set_key(keys, 5, 5, action); break;
		case SDLK_PERIOD: set_key(keys, 5, 4, action); break;
		case SDLK_SLASH: set_key(keys, 6, 7, action); break;
		case SDLK_COMMA: set_key(keys, 5, 7, action); break;
		case SDLK_SPACE: set_key(keys, 7, 4, action); break;
		case SDLK_SEMICOLON: set_key(keys, 6, 2, action); break;
		default:	break;
	}
}
