#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	_CIA *cia1 = (_CIA*)bus->cia1;
	_CIA *cia2 = (_CIA*)bus->cia2;

	_keymap	*keys = NULL;
	if (cia1 && cia1->keys)
		keys = (_keymap*)cia1->keys;
	
	uint8_t temp = 0;

	switch (addr) {
		case RASTER:
			return ((_VIC_II*)bus->vic)->raster & 0xFF;
		case CNTRL1:
			if ((((_VIC_II*)bus->vic)->raster >> 0x7) & 0x1)
				return bus->RAM[addr] | 0x1;
			return bus->RAM[addr] & ~0x1;
		
		case 0xDC04: /* CIA#1 TA LB */ return cia1->timerA & 0xFF;
		case 0xDC05: /* CIA#1 TA HB */ return (cia1->timerA >> 0x8) & 0xFF;
		case 0xDC06: /* CIA#1 TB LB */ return cia1->timerB & 0xFF;
		case 0xDC07: /* CIA#1 TB HB */ return (cia1->timerB >> 0x8) & 0xFF;
		case 0xDD04: /* CIA#2 TA LB */ return cia2->timerA & 0xFF;
		case 0xDD05: /* CIA#2 TA HB */ return (cia2->timerA >> 0x8) & 0xFF;
		case 0xDD06: /* CIA#2 TB LB */ return cia2->timerB & 0xFF;
		case 0xDD07: /* CIA#2 TB HB */ return (cia2->timerB >> 0x8) & 0xFF;
		case 0xDC0E: /* CIA#1 TA CTRL */ return temp;
		case 0xDC0F: /* CIA#1 TB CTRL */ return temp;
		case 0xDD0E: /* CIA#2 TA CTRL */ return temp;
		case 0xDD0F: /* CIA#2 TB CTRL */ return temp;
		case 0xDC0D:
			if (cia1->TA_interrupt_triggered || cia1->TB_interrupt_triggered) {
				temp |= 0x80;
				if (cia1->TA_interrupt_triggered) temp |= 0x1;
				if (cia1->TB_interrupt_triggered) temp |= 0x2;
			}
			return temp;
		case 0xDD0D:
			if (cia2->TA_interrupt_triggered || cia2->TB_interrupt_triggered) {
				temp |= 0x80;
				if (cia2->TA_interrupt_triggered) temp |= 0x1;
				if (cia2->TB_interrupt_triggered) temp |= 0x2;
			}
			return temp;
		case 0xDC00: return temp;
		case 0xDC01:
			if (!keys->active_row || keys->active_row == 0xFF)
				temp = keys->active_row;
			else {
				uint8_t row_i = 0;
				for (; row_i < 0x8 && ((keys->active_row >> row_i) & 0x1) != 0; row_i++);
				temp = keys->matrix[row_i];
			}
			return temp;
		default:	break;
	}

	return bus->RAM[addr];
}

void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_VIC_II *vic = (_VIC_II*)bus->vic;
	_CIA* cia1 = (_CIA*)bus->cia1;
	_CIA* cia2 = (_CIA*)bus->cia2;

	// Mirror $DCxx/$DDxx -> $DC0x/DD0x
	if ((addr >= CIA1_START && addr <= CIA1_END)
		|| (addr >= CIA2_START && addr <= CIA2_END)) {
		uint8_t low_nibble = addr & 0xF;
		addr = ((addr >> 0x8) & 0xFF) << 0x8 | low_nibble;
	}

	switch (addr) {
		case MEM_SETUP: // D018
			if ((bus->cpu_read(bus, CNTRL1) >> 0x5) & 0x1)
				vic->bitmap_ram = (val & 0x8) ? 0x2000 : 0x0000;
			switch ((val & 0xF) >> 0x1) {
				case 0x0:	vic->char_ram = 0x0; break;
				case 0x1:	vic->char_ram = 0x0800; break;
				case 0x2:	vic->char_ram = 0x1000; break;
				case 0x3:	vic->char_ram = 0x1800; break;
				case 0x4:	vic->char_ram = 0x2000; break;
				case 0x5:	vic->char_ram = 0x2800; break;
				case 0x6:	vic->char_ram = 0x3000; break;
				case 0x7:	vic->char_ram = 0x3800; break;
				default:	break;
			}
			switch (val >> 0x4) {
				case 0x0:	vic->screen_ram = 0x0; break;
				case 0x1:	vic->screen_ram = 0x0400; break;
				case 0x2:	vic->screen_ram = 0x0800; break;
				case 0x3:	vic->screen_ram = 0x0C00; break;
				case 0x4:	vic->screen_ram = 0x1000; break;
				case 0x5:	vic->screen_ram = 0x1400; break;
				case 0x6:	vic->screen_ram = 0x1800; break;
				case 0x7:	vic->screen_ram = 0x1C00; break;
				case 0x8:	vic->screen_ram = 0x2000; break;
				case 0x9:	vic->screen_ram = 0x2400; break;
				case 0xA:	vic->screen_ram = 0x2800; break;
				case 0xB:	vic->screen_ram = 0x2C00; break;
				case 0xC:	vic->screen_ram = 0x3000; break;
				case 0xD:	vic->screen_ram = 0x3400; break;
				case 0xE:	vic->screen_ram = 0x3800; break;
				case 0xF:	vic->screen_ram = 0x3C00; break;
				default:	break;
			}
			break;
		case CIA2_START: // DD00
			switch (val & 0x3) {
				case 0x0:	vic->bank = VIC_BANK_3; break;
				case 0x1:	vic->bank = VIC_BANK_2; break;
				case 0x2:	vic->bank = VIC_BANK_1; break;
				case 0x3:	vic->bank = VIC_BANK_0; break;
				default:	break;
			}
			break;

		case 0xDC00: /* CIA#1 PORT#A */
			((_keymap*)cia1->keys)->active_row = val;
			break;

		case 0xDC04: /* CIA#1 TA LB */ cia1->TA_latch_low = val; break;
		case 0xDC06: /* CIA#1 TB LB */ cia1->TB_latch_low = val; break;
		case 0xDD04: /* CIA#2 TA LB */ cia2->TA_latch_low = val; break;
		case 0xDD06: /* CIA#2 TB LB */ cia2->TB_latch_low = val; break;
		case 0xDC05: /* CIA#1 TA HB */ cia1->TA_latch_high = val; break;
		case 0xDC07: /* CIA#1 TB HB */ cia1->TB_latch_high = val; break;
		case 0xDD05: /* CIA#2 TA HB */ cia2->TA_latch_high = val; break;
		case 0xDD07: /* CIA#2 TB HB */ cia2->TB_latch_high = val; break;

		case 0xDC0E: /* CIA#1 TA CTRL */
			if ((val >> 0x4) & 0x1) { // bit#4 force reload
				cia1->timerA = cia1->TA_latch_high << 0x8 | cia1->TA_latch_low;
				// bit#3 restart/one-shot
				cia1->TA_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
			}
			cia1->TA_enable = val & 0x1; // bit#0 Start/Stop
			cia1->TA_input_mode = (val >> 0x5) & 0x1; // bit#5 input_mode
			bus->RAM[addr] = val;
			break;
		case 0xDC0F: /* CIA#1 TB CTRL */ 
			if ((val >> 0x4) & 0x1) {
				cia1->timerB = cia1->TB_latch_high << 0x8 | cia1->TB_latch_low;
				cia1->TB_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
			}
			cia1->TB_enable = val & 0x1;
			cia1->TB_input_mode = (val >> 0x5) & 0x1;
			bus->RAM[addr] = val;
			break;
		case 0xDD0E: /* CIA#2 TA CTRL */
			if ((val >> 0x4) & 0x1) {
				cia2->timerA = cia2->TA_latch_high << 0x8 | cia2->TA_latch_low;
				cia2->TA_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
			}
			cia2->TA_enable = val & 0x1;
			cia2->TA_input_mode = (val >> 0x5) & 0x1;
			bus->RAM[addr] = val;
			break;
		case 0xDD0F: /* CIA#2 TB CTRL */
			if ((val >> 0x4) & 0x1) {
				cia2->timerB = cia2->TB_latch_high << 0x8 | cia2->TB_latch_low;
				cia2->TB_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
			}
			cia2->TB_enable = val & 0x1;
			cia2->TB_input_mode = (val >> 0x5) & 0x1;
			bus->RAM[addr] = val;
			break;
		case 0xDC0D: /* CIA#1 INTR CTRL */
			if (val & 0x1) cia1->TA_interrupt_enable = (val >> 0x7) & 0x1;
			if ((val >> 0x1) & 0x1) cia1->TB_interrupt_enable = (val >> 0x7) & 0x1;
			bus->RAM[addr] = val;
			break;
		case 0xDD0D: /* CIA#2 INTR CTRL */
			if (val & 0x1) cia2->TA_interrupt_enable = (val >> 0x7) & 0x1;
			if ((val >> 0x1) & 0x1) cia2->TB_interrupt_enable = (val >> 0x7) & 0x1;
			bus->RAM[addr] = val;
			break;

		/*
			protected ZERO PAGE variables
		*/
		// BASIC top RAM / PRG max addr
		// for some reason kernal sets this to another addr
		case 0x37: bus->RAM[addr] = 0x00; return;
		case 0x38: bus->RAM[addr] = 0xA0; return;

		default:	break;
	}
	bus->RAM[addr] = val;
}

uint8_t	vic_read_(_bus *bus, uint16_t addr) {
	_VIC_II	*vic = (_VIC_II*)bus->vic;
	return bus->RAM[vic->bank + addr];
}

void	vic_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_VIC_II	*vic = (_VIC_II*)bus->vic;
	bus->RAM[vic->bank + addr] = val;
}

uint8_t	load_basic(_bus *bus) {
	char buffer[BASIC_ROM_SIZE];
	unsigned chars_read;
	FILE *file = fopen(BASIC_PATH, "rb");
	if (!file)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, BASIC_ROM_SIZE, file);
	if (!chars_read || chars_read != BASIC_ROM_SIZE) {
		fclose(file);
		return 0;
	}

	memcpy(bus->RAM + BASIC_ROM_START, buffer, BASIC_ROM_SIZE);
	fclose(file);
	return 1;
}

uint8_t	load_kernal(_bus *bus) {
	char buffer[KERNAL_ROM_SIZE];
	unsigned chars_read;
	FILE *file = fopen(KERNAL_PATH, "rb");
	if (!file)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, KERNAL_ROM_SIZE, file);
	if (!chars_read || chars_read != KERNAL_ROM_SIZE) {
		fclose(file);
		return 0;
	}

	memcpy(bus->RAM + KERNAL_ROM_START, buffer, KERNAL_ROM_SIZE);
	printf("kernal interrupt vectors: NMI: %04X, RSTV: %04X, IRQ: %04X\n",
		bus->cpu_read(bus, NMI + 1) << 0x8 | bus->cpu_read(bus, NMI),
		bus->cpu_read(bus, RSTV + 1) << 0x8 | bus->cpu_read(bus, RSTV),
		bus->cpu_read(bus, IRQ_BRK + 1) << 0x8 | bus->cpu_read(bus, IRQ_BRK));
	fclose(file);
	return 1;
}

uint8_t	load_chars(_bus *bus) {
	char buffer[CHAR_ROM_SIZE];
	unsigned chars_read;
	FILE *file = fopen(CHAR_ROM_PATH, "rb");
	if (!file)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, CHAR_ROM_SIZE, file);
	if (!chars_read || chars_read != CHAR_ROM_SIZE) {
		fclose(file);
		return 0;
	}

	/*memcpy(bus->RAM + UPP_CHAR_ROM_START, buffer, CHAR_ROM_SIZE);
	memcpy(bus->RAM + LOW_CHAR_ROM_START, buffer, CHAR_ROM_SIZE);*/
	memcpy(bus->RAM + 0xC000, buffer, CHAR_ROM_SIZE);
	fclose(file);
	return 1;
}

uint8_t	load_roms(_bus *bus) {
	// / ///		BASIC
	if (!load_basic(bus)) {
		printf("%sfailed to load BASIC ROM: %s%s\n%sexiting..%s\n",
			RED, RST, BASIC_PATH, RED, RST);
		return 0;
	}

	/// / //		KERNAL
	if (!load_kernal(bus)) {
		printf("%sfailed to load kernal:%s %s\n%sexiting..%s\n",
			RED, RST, KERNAL_PATH, RED, RST);
		return 0;
	}

	//// / //		CHARACTERS ROM
	if (!load_chars(bus)) {
		printf("%sfailed to load characters ROM:%s%s\n%sexiting..%s\n",
			RED, RST, CHAR_ROM_PATH, RED, RST);
		return 0;
	}
	return 1;
}

void	bus_init(_bus *bus) {
	memset(bus, 0, sizeof(_bus));
	bus->reset = bus_init;
	bus->cpu_read = cpu_read_;
	bus->cpu_write = cpu_write_;
	bus->vic_read = vic_read_;
	bus->vic_write = vic_write_;
	bus->load_roms = load_roms;
}

