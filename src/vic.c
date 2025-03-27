#include "metallc64.h"

uint16_t	get_raster(_VIC_II *vic) {
	uint8_t cntrl1 = vic->bus->ram_read(vic->bus, CNTRL1);
	return ((cntrl1 >> 0x7) & 0x1) << 0x8 |
		vic->bus->ram_read(vic->bus, RASTER);
}

void	increment_raster(_VIC_II *vic, uint16_t raster) {
	raster++;
	if (raster > GHEIGHT)
		raster = 0;
	vic->bus->ram_write(vic->bus, RASTER, raster & 0x00FF);
	uint8_t high_byte = (raster >> 0x8) & 0x1;
	uint8_t cntrl1 = vic->bus->ram_read(vic->bus, CNTRL1);
	if (high_byte)
		cntrl1 |= 0x80;
	else	cntrl1 &= ~0x80;
	vic->bus->ram_write(vic->bus, CNTRL1, cntrl1);
}
/*
   not the original C64 color pallete
   */
uint32_t	C64_to_rgb(uint8_t color) {
	uint32_t c64_colors[16] = {
		0x000000, 0xFFFFFF,
		0x68372B, 0x70A4B2,
		0x6F3D86, 0x588D43,
		0x352879, 0xB8C76F,
		0x6F4F25, 0x433900,
		0x9A6759, 0x444444,
		0x6C6C6C, 0x9AD284,
		0x6C5EB5, 0x959595,
	};
	return c64_colors[color & 0xF] << 0x8 | 0xFF;
}


void	draw_background_raster(_VIC_II *vic, uint32_t brd_col, uint32_t bg_col) {
	for (unsigned x = 0; x < 504; x++)
		put_pixel(vic, x, vic->raster,
				(x < 92 || x >= 412) ?
				brd_col :
				bg_col);
}

uint8_t vic_read_memory(_bus *bus, _VIC_II *vic, uint16_t addr) {
	uint16_t new_addr = addr + (vic->bank * VIC_BANK_SIZE);

	if (vic->char_rom_on && new_addr >= vic->char_ram
			&& new_addr < vic->char_ram + CHAR_ROM_SIZE)
		return bus->CHARACTERS[new_addr & 0x0FFF];

	return vic->vic_memory[vic->bank][addr];
}

void	vic_advance_raster(_bus *bus, _VIC_II *vic, unsigned cpu_cycles) {
	uint32_t bg_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR0));
	bool bitmap = (vic->control1 >> 0x5) & 0x1;
	bool extended = (vic->control1 >> 0x6) & 0x1;
	bool multicolor = (vic->control2 >> 0x4) & 0x1;
	bool visible_screen = (vic->control1 >> 0x4) & 0x1;
	bool cpu_irq_disable = ((_6502*)bus->cpu)->get_flag((_6502*)bus->cpu, 'I');
	uint8_t y_scroll = vic->control1 & 0x7;
	uint8_t x_scroll = vic->control2 & 0x7;
	unsigned screen_grid_h = ((vic->control1 >> 0x3) & 0x1) ? 25 : 24;
	unsigned screen_grid_w = ((vic->control2 >> 0x3) & 0x1) ? 40 : 38;
	unsigned screen_pixel_h = screen_grid_h * 8;
	unsigned screen_pixel_w = screen_grid_w * 8;
	unsigned screen_pixel_start_x = (GWIDTH - screen_pixel_w) / 2;
	unsigned screen_pixel_start_y = (GHEIGHT - screen_pixel_h) / 2;
	unsigned pixel_x, pixel_y;
	unsigned grid_x, grid_y, grid_pos;
	uint32_t fg_color, pixel_color;
	uint8_t screen_ram, char_data, bitmap_data;
	uint8_t bit_pos, color_pair, bg_index;
	uint8_t sprite_pixel;
	uint8_t sp_y, sp_h, sp_w;
	uint8_t sp_line, sp_data;
	uint16_t sp_x, sp_addr;
	bool sp_behind;
	/*
	   for collision detection
	   screen_line - screen/fourground x pixels
	   sprite_line - sprites non-transparent x pixels
	   */
	bool screen_line[GWIDTH];
	bool sprite_line[GWIDTH];

	//TODO:debug
	y_scroll = 0;
	x_scroll = 0;

	vic->cycles += cpu_cycles;
	for (; vic->cycles >= VIC_CYCLES_PER_LINE; vic->cycles -= VIC_CYCLES_PER_LINE) {
		memset(screen_line, 0, sizeof(screen_line));
		memset(sprite_line, 0, sizeof(sprite_line));
		if (visible_screen && vic->raster > DYSTART && vic->raster < DYEND) {
			for (unsigned x = DXSTART; x < DXEND; x++) {
				pixel_x = (x - screen_pixel_start_x + x_scroll) % screen_pixel_w;
				pixel_y = (vic->raster - screen_pixel_start_y + y_scroll) % screen_pixel_h;
				grid_x = (pixel_x / 8) % screen_grid_w;
				grid_y = (pixel_y / 8) % screen_grid_h;
				grid_pos = grid_y * screen_grid_w + grid_x;
				bit_pos = 7 - (pixel_x % 8);
				/* fine boundaries check */
				if (pixel_x >= DXEND || pixel_y >= DYEND)
					pixel_color = bg_color; //brd_color;
				else {
					//
					screen_ram = vic_read_memory(bus, vic, vic->screen_ram + grid_pos);
					//
					if (bitmap) {
						uint32_t bitmap_offset = ((pixel_y / 8) * screen_grid_w + (pixel_x / 8)) * 8;
						bitmap_data = vic_read_memory(bus, vic, vic->bitmap_ram + bitmap_offset + (pixel_y % 8));
						if (multicolor && !extended) {
							/* multicolor bitmap mode */
							bit_pos = 6 - (((pixel_x >> 1) & 3) * 2);
							color_pair = (bitmap_data >> bit_pos) & 0x3;
							switch (color_pair) {
								case 0: /* 00 */ pixel_color = bg_color; break;
								case 1: /* 01 */ pixel_color = vic->C64_to_rgb((screen_ram >> 4) & 0xF); break;
								case 2: /* 10 */ pixel_color = vic->C64_to_rgb(screen_ram & 0xF); break;
								case 3: /* 11 */ pixel_color = vic->C64_to_rgb(bus->ram_read(bus, VIC_COLOR_START + grid_pos)); break;
							}
							screen_line[x] = color_pair != 0;
						}
						else {
							/* standard bitmap mode */
							if ((bitmap_data >> bit_pos) & 0x1)
								pixel_color = vic->C64_to_rgb((screen_ram >> 4) & 0xF);
							else	pixel_color = vic->C64_to_rgb(screen_ram & 0xF);
							screen_line[x] = TRUE;
						}
					}
					else {
						if (extended && !multicolor) {
							/* extended background color mode */
							char_data = vic_read_memory(bus, vic,
									vic->char_ram + ((screen_ram & 0x3F) * 8) + (pixel_y % 8));
							bg_index = (screen_ram >> 0x6) & 0x3;
							bg_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR0 + bg_index));
							fg_color = vic->C64_to_rgb(bus->ram_read(bus, VIC_COLOR_START + grid_pos));
							if ((char_data >> bit_pos) & 0x1) {
								pixel_color = fg_color;
								screen_line[x] = TRUE;
							}
							else pixel_color = bg_color;
						}
						else {
							char_data = vic_read_memory(bus, vic,
									vic->char_ram + (screen_ram * 8) + (pixel_y % 8));
							fg_color = vic->C64_to_rgb(bus->ram_read(bus, VIC_COLOR_START + grid_pos));
							if (multicolor) {
								/* multicolor character mode */
								bit_pos = 6 - (((pixel_x >> 1) & 3) * 2);
								color_pair =  (char_data >> bit_pos) & 0x3;
								switch (color_pair) {
									case 0: /* 00 */ pixel_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR0)); break;
									case 1: /* 01 */ pixel_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR1)); break;
									case 2: /* 10 */ pixel_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR2)); break;
									case 3: /* 11 */ pixel_color = fg_color; break;
								}
								screen_line[x] = color_pair == 0x3;
							}
							else {
								/* standard character mode */
								bg_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR0));
								if ((char_data >> bit_pos) & 0x1) {
									pixel_color = fg_color;
									screen_line[x] = TRUE;
								}
								else	pixel_color = bg_color;
							}
						}
					}
				}
				put_pixel(vic, x, vic->raster, pixel_color);
			}
		}
		/* sprites TODO:fix */
		if (vic->sprite_enable) {
			for (unsigned sp_i = 0; sp_i < 0x8; sp_i++) {
				if ((vic->sprite_enable >> sp_i) & 0x1) {
					/* sprite is enabled */
					sp_h = ((vic->sprite_expand_y >> sp_i) & 0x1) ? SPRT_H*2 : SPRT_H;
					sp_w = ((vic->sprite_expand_x >> sp_i) & 0x1) ? SPRT_W*2 : SPRT_W;
					sp_y = vic->sprite_y[sp_i];
					if (vic->raster < sp_y || vic->raster >= sp_y + sp_h)
						continue;
					/* sprite is in current raster */
					sp_addr = 64 * vic_read_memory(bus, vic, vic->screen_ram + SPRT_PTRS_ADDR + sp_i);
					sp_x = vic->sprite_x[sp_i];
					if ((vic->sprite_8x >> sp_i) & 0x1)
						sp_x |= 0x100;
					sp_behind = (vic->sprite_priority >> sp_i) & 0x1;
					multicolor = (vic->sprite_multicolor_enable >> sp_i) & 0x1;
					sp_line = vic->raster - sp_y;
					sp_data = vic_read_memory(bus, vic, sp_addr + sp_line);
					/* sprite row rendering */
					for (unsigned x = 0; x < GWIDTH; x++) {
						if (x >= sp_x && x < sp_x + sp_w) {
							pixel_x = x - sp_x;
							if ((vic->sprite_expand_x >> sp_i) & 0x1) 
								pixel_x /= 2;
							sprite_pixel = (sp_data >> (7 - pixel_x)) & 0x1;
							if (sprite_pixel) {
								if (multicolor) {
									bit_pos = 6 - (((pixel_x >> 1) & 3) * 2);
									sprite_pixel = (sp_data >> bit_pos) & 0x3;
									switch (sprite_pixel) {
										case 0x0: /* 00 */ continue; // Transparent
										case 0x1: /* 01 */ pixel_color = vic->C64_to_rgb(vic->sprite_multicolor0); break;
										case 0x2: /* 10 */ pixel_color = vic->C64_to_rgb(vic->sprite_multicolor1); break;
										case 0x3: /* 11 */ pixel_color = vic->C64_to_rgb(vic->sprite_colors[sp_i]); break;
									}
								}
								else	pixel_color = vic->C64_to_rgb(vic->sprite_colors[sp_i]);
								/* priority check */
								if (!screen_line[x] || !sp_behind)
									put_pixel(vic, x, vic->raster, pixel_color);
								/* fine collision detection */
								if (screen_line[x]) {
									vic->sp_bg_collision |= (1 << sp_i);
									if (vic->sp_bg_interrupt_enable) {
										vic->sp_bg_interrupt_triggered = TRUE;
										if (!cpu_irq_disable) ((_6502*)bus->cpu)->irq_pending = TRUE;
									}
								}
								if (sprite_line[x]) {
									vic->sp_sp_collision |= (1 << sp_i);
									if (vic->sp_sp_interrupt_enable) {
										vic->sp_sp_interrupt_triggered = TRUE;
										if (!cpu_irq_disable) ((_6502*)bus->cpu)->irq_pending = TRUE;
									}
								}
								sprite_line[x] = TRUE;
							}
						}
					}
				}
			}
		}
		/* raster interrupt */
		vic->raster++;
		if (vic->raster == vic->get_raster(vic)) {
			if (vic->raster_interrupt_enable) {
				vic->raster_interrupt_triggered = TRUE;
				if (!cpu_irq_disable)
					((_6502*)bus->cpu)->irq_pending = TRUE;
			}
		}
		/* raster reset */
		if (vic->raster >= GHEIGHT)
			vic->raster = 0;
	}
}

_VIC_II	*vic_init(_bus *bus) {
	_VIC_II *vic = malloc(sizeof(_VIC_II));
	if (!vic) {
		free(vic);
		free(bus->cpu);
		return FALSE;
	}
	memset(vic, 0, sizeof(_VIC_II));
	vic->init = vic_init;
	vic->get_raster = get_raster;
	vic->increment_raster = increment_raster;
	vic->C64_to_rgb = C64_to_rgb;
	vic->bus = bus;
	vic->screen_ram = DEFAULT_SCREEN;
	vic->char_ram = LOW_CHAR_ROM_START;
	vic->sprite_ptrs = SPRT_PTRS_ADDR;
	vic->bank = VIC_BANK_0;
	vic->bitmap_ram = 0x00;
	vic->vic_memory[0] = &bus->RAM[0x0000];
	vic->vic_memory[1] = &bus->RAM[0x4000];
	vic->vic_memory[2] = &bus->RAM[0x8000];
	vic->vic_memory[3] = &bus->RAM[0xC000];
	return vic;
}

