#include "metallc64.h"

uint16_t	get_raster(_VIC_II *vic) {
	uint8_t cntrl1 = vic->bus->cpu_read(vic->bus, CNTRL1);
	return ((cntrl1 >> 0x7) & 0x1) << 0x8 |
		vic->bus->cpu_read(vic->bus, RASTER);
}

void	increment_raster(_VIC_II *vic, uint16_t raster) {
	raster++;
	if (raster > GHEIGHT)
		raster = 0;
	vic->bus->cpu_write(vic->bus, RASTER, raster & 0x00FF);
	uint8_t high_byte = (raster >> 0x8) & 0x1;
	uint8_t cntrl1 = vic->bus->cpu_read(vic->bus, CNTRL1);
	if (high_byte)
		cntrl1 |= 0x80;
	else	cntrl1 &= ~0x80;
	vic->bus->cpu_write(vic->bus, CNTRL1, cntrl1);
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

uint8_t vic_read_memory(_VIC_II *vic, uint16_t addr) {
	uint16_t new_addr = addr + (vic->bank * VIC_BANK_SIZE);

	if ((new_addr >= 0x1000 && new_addr < 0x2000)
		&& (vic->bank == VIC_BANK_0 || vic->bank == VIC_BANK_2))
		return vic->char_rom[new_addr & 0x0FFF];

	return vic->vic_memory[vic->bank][addr];
}

void vic_advance_raster(_bus *bus, _VIC_II *vic, unsigned cpu_cycles) {
	uint32_t pixel_color;
	unsigned char_x, char_y, char_grid_pos;
	uint8_t control1, control2;

	vic->cycles += cpu_cycles;
	for (; vic->cycles >= VIC_CYCLES_PER_LINE; vic->cycles -= VIC_CYCLES_PER_LINE) {
		control1 = bus->cpu_read(bus, CNTRL1);
		control2 = bus->cpu_read(bus, CNTRL2);

		uint8_t bitmap_mode = (control1 >> 5) & 1;
		uint8_t extended_mode = (control1 >> 6) & 1;
		uint8_t multicolor_mode = (control2 >> 4) & 1;

		uint32_t border_color = vic->C64_to_rgb(bus->cpu_read(bus, BRD_COLOR));
		uint32_t bg_color[4] = {
			vic->C64_to_rgb(bus->cpu_read(bus, BACKG_COLOR0)),
			vic->C64_to_rgb(bus->cpu_read(bus, BACKG_COLOR1)),
			vic->C64_to_rgb(bus->cpu_read(bus, BACKG_COLOR2)),
			vic->C64_to_rgb(bus->cpu_read(bus, BACKG_COLOR3))
		};

		// Border area
		if (vic->raster < DYSTART || vic->raster >= DYEND
			|| !((control1 >> 4) & 1)) {
			for (unsigned x = 0; x < GWIDTH; x++)
				put_pixel(vic, x, vic->raster, border_color);
		}
		else {
			for (unsigned x = 0; x < GWIDTH; x++) {
				if (x < DXSTART || x >= DXEND) {
					put_pixel(vic, x, vic->raster, border_color);
					continue;
				}
				char_x = (x - DXSTART) / 8;
				char_y = (vic->raster - DYSTART) / 8;
				char_grid_pos = char_y * 40 + char_x;
				uint8_t x_pixel = (x - DXSTART) % 8;
				uint8_t y_pixel = (vic->raster - DYSTART) % 8;

				uint8_t color_ram = bus->cpu_read(bus, VIC_COLOR_START + char_grid_pos);
				uint8_t screen_ram = vic_read_memory(vic, vic->screen_ram + char_grid_pos);
				uint32_t fg_color = vic->C64_to_rgb(color_ram & 0xF);

				if (bitmap_mode) {
					// Bitmap mode
					uint8_t byte_offset = (char_y * 320) + (char_x * 8) + y_pixel;
					uint8_t bitmap_data = vic_read_memory(vic, vic->bitmap_ram + byte_offset);

					if (multicolor_mode && (color_ram & 0x8)) {
						uint8_t color_pair = (bitmap_data >> (6 - (x_pixel & 0x6))) & 3;
						switch (color_pair) {
							case 0: pixel_color = bg_color[0]; break;
							case 1: pixel_color = vic->C64_to_rgb(screen_ram & 0xF); break;
							case 2: pixel_color = vic->C64_to_rgb(screen_ram >> 4); break;
							case 3: pixel_color = fg_color; break;
						}
					} else {
						pixel_color = bitmap_data & (0x80 >> x_pixel) ?
							vic->C64_to_rgb(screen_ram >> 4) :
							vic->C64_to_rgb(screen_ram & 0xF);
					}
				} else {
					// Character modes
					//uint8_t char_data = vic->char_rom[(screen_ram * 8) + y_pixel];
					uint8_t char_data = vic_read_memory(vic, vic->char_ram + ((screen_ram * 8) + y_pixel));
					if (multicolor_mode && (color_ram & 0x8)) {
						uint8_t color_pair = (char_data >> (6 - (x_pixel & 0x6))) & 3;
						switch (color_pair) {
							case 0: pixel_color = bg_color[0]; break;
							case 1: pixel_color = bg_color[1]; break;
							case 2: pixel_color = fg_color; break;
							case 3: pixel_color = bg_color[2]; break;
						}
					} else if (extended_mode) {
						uint8_t bg_index = (screen_ram >> 6) & 3;
						pixel_color = char_data & (0x80 >> x_pixel) ?
							fg_color : bg_color[bg_index];
					} else {
						pixel_color = char_data & (0x80 >> x_pixel) ?
							fg_color : bg_color[0];
					}
				}

				put_pixel(vic, x, vic->raster, pixel_color);
			}
		}

		// Raster interrupt handling
		vic->raster++;
		if (vic->raster == vic->get_raster(vic)) {
			uint8_t D019 = bus->cpu_read(bus, INTR_STATUS);
			bus->cpu_write(bus, INTR_STATUS, D019 & ~0x1);
			if ((bus->cpu_read(bus, INTR_ON) & 0x1) && 
					!((_6502*)bus->cpu)->get_flag((_6502*)bus->cpu, 'I')) {
				((_6502*)bus->cpu)->irq_pending = 1;
			}
		}
		if (vic->raster > GHEIGHT)
			vic->raster = 0;
	}
}

void	vic_init(_bus *bus, _VIC_II *vic) {
	memset(vic, 0, sizeof(_VIC_II));
	vic->init = vic_init;
	vic->get_raster = get_raster;
	vic->increment_raster = increment_raster;
	vic->C64_to_rgb = C64_to_rgb;
	vic->bus = bus;
	vic->screen_ram = DEFAULT_SCREEN;
	vic->char_ram = LOW_CHAR_ROM_START;
	vic->bitmap_ram = 0x000;
	vic->bank = VIC_BANK_0;
	vic->vic_memory[0] = &bus->RAM[0x0000];
	vic->vic_memory[1] = &bus->RAM[0x4000];
	vic->vic_memory[2] = &bus->RAM[0x8000];
	vic->vic_memory[3] = &bus->RAM[0xC000];
	memcpy(vic->char_rom, bus->RAM + LOW_CHAR_ROM_START, CHAR_ROM_SIZE);
}

