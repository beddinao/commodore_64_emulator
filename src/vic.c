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

void	vic_advance_raster(_bus *bus, _VIC_II *vic, unsigned cpu_cycles) {
	uint32_t brd_color, bg_color, fg_color, t_bg_color;
	vic->cycles += cpu_cycles;
	for (; vic->cycles >= VIC_CYCLES_PER_LINE; vic->cycles -= VIC_CYCLES_PER_LINE) {

		brd_color = vic->C64_to_rgb(bus->vic_read(bus, BRD_COLOR));
		bg_color = vic->C64_to_rgb(bus->vic_read(bus, BACKG_COLOR0));

		if (vic->raster < DYSTART || vic->raster >= DYEND) {
			for (unsigned x = 0; x < GWIDTH; x++)
				put_pixel(vic, x, vic->raster, brd_color);
		}
		else {
			for (unsigned x = 0, row, col; x < GWIDTH; x++) {
				if (x < DXSTART || x >= DXEND)
					put_pixel(vic, x, vic->raster, brd_color);
				else {
					col = (x - DXSTART) / 0x8;
					row = (vic->raster - DYSTART) / 0x8;
					if ((bus->cpu_read(bus, CNTRL1) >> 0x5) & 0x1) {
						uint8_t byte_offset = (row * 320) + (col * 8) + (vic->raster % 8);
						uint8_t bitmap_data = bus->vic_read(bus, vic->bitmap_ram + byte_offset);
						uint8_t color_data = bus->vic_read(bus, vic->screen_ram + (row * 40 + col));
						uint8_t bit_pos = (x - DXSTART) % 0x8;
						if (bitmap_data & (0x80 >> bit_pos))
							fg_color = vic->C64_to_rgb(color_data >> 0x4);
						else	fg_color = vic->C64_to_rgb(color_data & 0xF);
						put_pixel(vic, x, vic->raster, fg_color);
					}
					else {
						fg_color = vic->C64_to_rgb(bus->vic_read(bus, VIC_COLOR_START + (row * 40 + col)));
						uint8_t char_code = bus->vic_read(bus, vic->screen_ram + (row * 40 + col));
						uint8_t pixel_data = bus->vic_read(bus, 0xC000/*vic->char_ram*/ + (char_code * 0x8) + (vic->raster % 0x8));
						uint8_t bit_pos = (x - DXSTART) % 0x8;
						if ((bus->cpu_read(bus, CNTRL1) >> 0x6) & 0x1) {
							uint8_t bg_index = (char_code >> 0x6) & 0x3;
							t_bg_color = vic->C64_to_rgb(bus->vic_read(bus, BACKG_COLOR2 + bg_index));
						}
						else	t_bg_color = bg_color;
						put_pixel(vic, x, vic->raster, (pixel_data & (0x80 >> bit_pos)) ? fg_color : t_bg_color);
					}
				}
			}
		}
		vic->raster++;
		if (vic->raster == vic->get_raster(vic)) {
			uint8_t _D019 = bus->cpu_read(bus, INTR_STATUS);
			bus->cpu_write(bus, INTR_STATUS, _D019 & ~0x1);
			if ((bus->cpu_read(bus, INTR_ON) & 0x1) && !((_6502*)bus->cpu)->get_flag((_6502*)bus->cpu, 'I')) {
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
	vic->char_ram = LOW_CHAR_ROM_START;
	vic->screen_ram = DEFAULT_SCREEN;
	vic->bitmap_ram = 0x00;
	vic->bank = VIC_BANK_0;
}

