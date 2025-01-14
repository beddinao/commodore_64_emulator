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

uint32_t	C64_to_rgb(uint8_t color) {
	uint32_t c64_colors[16] = {
		0x000000,
		0xFFFFFF,
		0x68372B,
		0x70A4B2,
		0x6F3D86,
		0x588D43,
		0x352879,
		0xB8C76F,
		0x6F4F25,
		0x433900,
		0x9A6759,
		0x444444,
		0x6C6C6C,
		0x9AD284,
		0x6C5EB5,
		0x959595,
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

	if (new_addr >= vic->char_ram && new_addr < vic->char_ram + CHAR_ROM_SIZE
			&& (vic->bank == VIC_BANK_0 || vic->bank == VIC_BANK_2))
		return bus->CHARACTERS[new_addr & 0x0FFF];

	return vic->vic_memory[vic->bank][addr];
}

void	vic_advance_raster(_bus *bus, _VIC_II *vic, unsigned cpu_cycles) {
	uint32_t brd_color, bg_color, fg_color, pixel_color;
	uint8_t screen_ram, char_data, bitmap_data;
	uint8_t bit_pos, color_pair, bg_index;
	bool bitmap, extended, multicolor, visible_screen;
	unsigned pixel_x, pixel_y;
	unsigned grid_x, grid_y, grid_pos;
	unsigned screen_grid_h, screen_grid_w;
	unsigned screen_pixel_h, screen_pixel_w;
	unsigned screen_pixel_start_x, screen_pixel_start_y;
	unsigned screen_pixel_end_x, screen_pixel_end_y;
	uint8_t x_scroll, y_scroll;

	vic->cycles += cpu_cycles;
	for (; vic->cycles >= VIC_CYCLES_PER_LINE; vic->cycles -= VIC_CYCLES_PER_LINE) {

		brd_color = vic->C64_to_rgb(bus->ram_read(bus, BRD_COLOR));
		bg_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR0));

		bitmap = (vic->control1 >> 0x5) & 0x1;
		extended = (vic->control1 >> 0x6) & 0x1;
		multicolor = (vic->control2 >> 0x4) & 0x1;
		visible_screen = (vic->control1 >> 0x4) & 0x1;
		y_scroll = vic->control1 & 0x7;
		x_scroll = vic->control2 & 0x7;
		screen_grid_h = ((vic->control1 >> 0x3) & 0x1) ? 25 : 24;
		screen_grid_w = ((vic->control2 >> 0x3) & 0x1) ? 40 : 38;
		screen_pixel_h = screen_grid_h * 8;
		screen_pixel_w = screen_grid_w * 8;
		screen_pixel_start_x = (GWIDTH - screen_pixel_w) / 2;
		screen_pixel_start_y = (GHEIGHT - screen_pixel_h) / 2;
		screen_pixel_end_x = GWIDTH - screen_pixel_start_x;
		screen_pixel_end_y = GHEIGHT - screen_pixel_start_y;

		if (!visible_screen || vic->raster < DYSTART || vic->raster >= DYEND) {
			pixel_color = brd_color;
			for (unsigned x = 0; x < GWIDTH; x++)
				put_pixel(vic, x, vic->raster, pixel_color);
		}
		else {
			for (unsigned x = 0; x < GWIDTH; x++) {
				if (x < DXSTART || x >= DXEND)
					pixel_color = brd_color;
				else {
					pixel_x = (x - screen_pixel_start_x + x_scroll)/* % screen_pixel_w*/;
					pixel_y = (vic->raster - screen_pixel_start_y + y_scroll)/* % screen_pixel_h*/;
					grid_x = (pixel_x / 8) /*% screen_grid_w*/;
					grid_y = (pixel_y / 8) /*% screen_grid_h*/;
					grid_pos = grid_y * screen_grid_w + grid_x;
					bit_pos = 7 - (pixel_x % 8);
					/* fine boundaries check */
					if (pixel_x >= screen_pixel_w || pixel_y >= screen_pixel_h)
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
									case 0: pixel_color = bg_color; break;
									case 1: pixel_color = vic->C64_to_rgb(screen_ram & 0xF); break;
									case 2: pixel_color = vic->C64_to_rgb((screen_ram >> 4) & 0xF); break;
									case 3: pixel_color = vic->C64_to_rgb(bus->ram_read(bus, VIC_COLOR_START + grid_pos)); break;
								}
							}
							else {
								/* standard bitmap mode */
								if ((bitmap_data >> bit_pos) & 0x1)
									pixel_color = vic->C64_to_rgb((screen_ram >> 4) & 0xF);
								else	pixel_color = vic->C64_to_rgb(screen_ram & 0xF);
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
								pixel_color = ((char_data >> bit_pos) & 0x1) ? fg_color : bg_color;

							}
							else {
								char_data = vic_read_memory(bus, vic,
									vic->char_ram + (screen_ram * 8) + (pixel_y % 8));
								fg_color = vic->C64_to_rgb(bus->ram_read(bus, VIC_COLOR_START + grid_pos));
								if (multicolor) {
									/* multicolor character mode */
									//pixel_x = pixel_x / 2;
									bit_pos = 6 - (((pixel_x >> 1) & 3) * 2);
									color_pair =  (char_data >> bit_pos) & 0x3;
									switch (color_pair) {
										case 0: pixel_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR0)); break;
										case 1: pixel_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR1)); break;
										case 2: pixel_color = fg_color; break;
										case 3: pixel_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR2)); break;
									}
								}
								else {
									/* standard character mode */
									bg_color = vic->C64_to_rgb(bus->ram_read(bus, BACKG_COLOR0));
									pixel_color = ((char_data >> bit_pos) & 0x1) ? fg_color : bg_color;
								}
							}
						}
					}
				}
				put_pixel(vic, x, vic->raster, pixel_color);
			}
		}
		vic->raster++;
		if (vic->raster == vic->get_raster(vic)) {
			bus->ram_write(bus, INTR_STATUS, bus->ram_read(bus, INTR_STATUS) | 0x1);
			if ((bus->ram_read(bus, INTR_ON) & 0x1)
					&& !((_6502*)bus->cpu)->get_flag((_6502*)bus->cpu, 'I')) {
				((_6502*)bus->cpu)->irq_pending = 1;
			}
		}
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
	vic->bitmap_ram = 0x00;
	vic->bank = VIC_BANK_0;
	vic->vic_memory[0] = &bus->RAM[0x0000];
	vic->vic_memory[1] = &bus->RAM[0x4000];
	vic->vic_memory[2] = &bus->RAM[0x8000];
	vic->vic_memory[3] = &bus->RAM[0xC000];
	return vic;
}

