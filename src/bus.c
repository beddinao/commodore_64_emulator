#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	_CIA *cia1 = (_CIA*)bus->cia1;
	_CIA *cia2 = (_CIA*)bus->cia2;
	_VIC_II *vic = (_VIC_II*)bus->vic;
	_keymap *keys = (cia1 && cia1->keys) ? (_keymap*)cia1->keys : NULL;

	uint8_t proc_port = bus->RAM[1];
	uint8_t retuv = 0;

	bool basic_rom_enabled = (proc_port & 0x3) == 0x3; // $A000-$BFFF --xx
	bool kernal_rom_enabled = (proc_port & 0x2) == 0x2; // $E000-$DFFF --xx
	bool io_enabled = (proc_port & 0x4) == 0x4; // $D000-$DFFF -x--
	bool char_rom_enabled = (proc_port & 0x4) == 0x0; // $D000-$DFFF -x--

	/*  KERNAL */
	if (addr >= KERNAL_ROM_START && kernal_rom_enabled)
		return bus->RAM[addr];
	/*  BASIC  */
	if (addr >= BASIC_ROM_START && addr <= BASIC_ROM_END && basic_rom_enabled)
		return bus->RAM[addr];
	if (addr >= IO_CHAR_START && addr <= IO_CHAR_END) {
		if (!io_enabled) {
			/*  CHARACTERS  */
			if (char_rom_enabled)
				return vic->char_rom[addr - IO_CHAR_START];
			return bus->RAM[addr]; // whatever
		}
		/*  IO  */
		// VIC Registers
		if (addr >= VIC_REG_START && addr <= VIC_REG_END) {
			// Mirror $D000-$D3FF -> $D000-$D03F
			uint8_t low_nibble = addr & 0x3F;
			addr = ((addr >> 0x8) & 0xFF) << 0x8 | low_nibble;
			switch (addr) {
				case INTR_STATUS: // $D019
					retuv = bus->RAM[addr];
					bus->RAM[addr] &= ~0x1F;
					return retuv;
				case RASTER: // $D012
					return vic->raster & 0xFF;
				case CNTRL1: // $D011
					return ((vic->raster >> 0x7) & 0x1) ?
						(bus->RAM[addr] | 0x80):
						(bus->RAM[addr] & ~0x80);
				default:	return bus->RAM[addr];
			}
		}
		// CIAs Registers
		if ((addr >= CIA1_START && addr <= CIA1_END)
			|| (addr >= CIA2_START && addr <= CIA2_END)) {
			// Mirror $DCxx/$DDxx -> $DC0x/DD0x
			uint8_t low_nibble = addr & 0xF;
			addr = ((addr >> 0x8) & 0xFF) << 0x8 | low_nibble;

			switch (addr) {
				/*
					CIA 1 $DC0x
				*/
				case 0xDC00: return retuv;
				case 0xDC01:
					if (!keys->active_row || keys->active_row == 0xFF)
						retuv = keys->active_row;
					else {
						uint8_t row_i = 0;
						for (; row_i < 0x8 && ((keys->active_row >> row_i) & 0x1) != 0; row_i++);
						retuv = keys->matrix[row_i];
					}
					return retuv;
				case 0xDC04: /* timerA low */  return cia1->timerA & 0xFF;
				case 0xDC05: /* timerA high*/  return (cia1->timerA >> 0x8) & 0xFF;
				case 0xDC06: /* timerB low */  return cia1->timerB & 0xFF;
				case 0xDC07: /* timerB high*/  return (cia1->timerB >> 0x8) & 0xFF;
				case 0xDC0D: /* Interrupt Control */
					if (cia1->TA_interrupt_triggered || cia1->TB_interrupt_triggered) {
						retuv |= 0x80;
						if (cia1->TA_interrupt_triggered) retuv |= 0x1;
						if (cia1->TB_interrupt_triggered) retuv |= 0x2;
					}
					return retuv;
				case 0xDC0E: return retuv;
				case 0xDC0F: return retuv;
				/*
					CIA 2 $DD0x
				*/
				case 0xDD04: /* timerA low */ return cia2->timerA & 0xFF;
				case 0xDD05: /* timerA high */ return (cia2->timerA >> 0x8) & 0xFF;
				case 0xDD06: /* timerB low */ return cia2->timerB & 0xFF;
				case 0xDD07: /* timerB high */ return (cia2->timerB >> 0x8) & 0xFF;
				case 0xDD0D: /* Interrupt Control */
					if (cia2->TA_interrupt_triggered || cia2->TB_interrupt_triggered) {
						retuv |= 0x80;
						if (cia2->TA_interrupt_triggered) retuv |= 0x1;
						if (cia2->TB_interrupt_triggered) retuv |= 0x2;
					}
					return retuv;
				case 0xDD0E: return retuv;
				case 0xDD0F: return retuv;
				default:
					   return bus->RAM[addr];
			}
		}
	}
	return bus->RAM[addr];
}


void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_VIC_II *vic = (_VIC_II*)bus->vic;
	_CIA* cia1 = (_CIA*)bus->cia1;
	_CIA* cia2 = (_CIA*)bus->cia2;

	switch (addr) {
		case 0x01: bus->RAM[addr] = val; return;
		// protected: BASIC TOP RAM / PRG max addr
		//case 0x37: bus->RAM[addr] = 0x00; return;
		//case 0x38: bus->RAM[addr] = 0xA0; return;
	}

	uint8_t proc_port = bus->RAM[1];

	bool basic_rom_enabled = (proc_port & 0x3) == 0x3; // $A000-$BFFF --xx
	bool kernal_rom_enabled = (proc_port & 0x2) == 0x2; // $E000-$DFFF --xx
	bool io_enabled = (proc_port & 0x4) == 0x4; // $D000-$DFFF -x--
	//bool char_rom_enabled = (proc_port & 0x4) == 0x0; // $D000-$DFFF -x-

	/*  IO  */
	if (addr >= IO_CHAR_START && addr <= IO_CHAR_END && io_enabled) {
		// VIC Registers
		if (addr >= VIC_REG_START && addr <= VIC_REG_END) {
			// Mirror $D000-$D3FF -> $D000-$D03F
			uint8_t low_nibble = addr & 0x3F;
			addr = ((addr >> 0x8) & 0xFF) << 0x8 | low_nibble;
			switch (addr) {
				case MEM_SETUP: // $D018
					/*ignorant stuff
					vic->bitmap_ram = (val & 0x8) ? 0x2000 : 0x0000;
					switch ((val & 0xF) >> 0x1) {
						case 0x0:	vic->char_ram = 0x0000; break;
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
						case 0x0:	vic->screen_ram = 0x0000; break;
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
					}*/
					vic->char_ram = (val & 0xe) << 10;
					vic->screen_ram = (val & 0xf0) << 6;
					vic->bitmap_ram = (val & 0x8) << 10;
					bus->RAM[addr] |= 0x1;
				break;
			}
		}
		// CIAs Registers
		if ((addr >= CIA1_START && addr <= CIA1_END)
			|| (addr >= CIA2_START && addr <= CIA2_END)) {
			// Mirror $DCxx/$DDxx -> $DC0x/DD0x
			uint8_t low_nibble = addr & 0xF;
			addr = ((addr >> 0x8) & 0xFF) << 0x8 | low_nibble;
			
			switch (addr) {
				/*
					CIA 1 $DC0x
				*/
				case 0xDC00: // PORT#A Keyboard matrix
					((_keymap*)cia1->keys)->active_row = val;
					break;
				case 0xDC04: // TimerA low
					cia1->TA_latch_low = val;
					break;
				case 0xDC05: // TimerA high
					cia1->TA_latch_high = val;
					break;
				case 0xDC06: // TimerB low
					cia1->TB_latch_low = val;
					break;
				case 0xDC07: // TimerB high
					cia1->TB_latch_high = val;
					break;
				case 0xDC0D: // Interrupt control
					if (val & 0x1) cia1->TA_interrupt_enable = (val >> 0x7) & 0x1;
					if ((val >> 0x1) & 0x1) cia1->TB_interrupt_enable = (val >> 0x7) & 0x1;
					break;
				case 0xDC0E: // Timer A Control
					if ((val >> 0x4) & 0x1) { // bit#4 force reload
						cia1->timerA = cia1->TA_latch_high << 0x8 | cia1->TA_latch_low;
						// bit#3 restart/one-shot
						cia1->TA_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
					}
					cia1->TA_enable = val & 0x1; // bit#0 Start/Stop
					cia1->TA_input_mode = (val >> 0x5) & 0x1; // bit#5 input_mode
					break;
				case 0xDC0F: // Timer B Control
					if ((val >> 0x4) & 0x1) {
						cia1->timerB = cia1->TB_latch_high << 0x8 | cia1->TB_latch_low;
						cia1->TB_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
					}
					cia1->TB_enable = val & 0x1;
					cia1->TB_input_mode = (val >> 0x5) & 0x1;
					break;
				/*
					CIA 2 $DD0x
				*/
				case 0xDD00: // VIC Bank switching
					switch (val & 0x3) {
						case 0x0:	vic->bank = VIC_BANK_3; break;
						case 0x1:	vic->bank = VIC_BANK_2; break;
						case 0x2:	vic->bank = VIC_BANK_1; break;
						case 0x3:	vic->bank = VIC_BANK_0; break;
						default:	break;
					}
					break;
				case 0xDD04: // Timer A low
					cia2->TA_latch_low = val;
					break;
				case 0xDD05: // Timer A high
					cia2->TA_latch_high = val;
					break;
				case 0xDD06: // Timer B low
					cia2->TB_latch_low = val;
					break;
				case 0xDD07: // Timer B high
					cia2->TB_latch_high = val;
					break;
				case 0xDD0D: // Interrupt control 
					if (val & 0x1) cia2->TA_interrupt_enable = (val >> 0x7) & 0x1;
					if ((val >> 0x1) & 0x1) cia2->TB_interrupt_enable = (val >> 0x7) & 0x1;
					bus->RAM[addr] = val;
					break;
				case 0xDD0E: // Timer A control 
					if ((val >> 0x4) & 0x1) {
						cia2->timerA = cia2->TA_latch_high << 0x8 | cia2->TA_latch_low;
						cia2->TA_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
					}
					cia2->TA_enable = val & 0x1;
					cia2->TA_input_mode = (val >> 0x5) & 0x1;
					break;
				case 0xDD0F: // Timer B control 
					if ((val >> 0x4) & 0x1) {
						cia2->timerB = cia2->TB_latch_high << 0x8 | cia2->TB_latch_low;
						cia2->TB_mode = ((bus->RAM[addr] >> 0x3) & 0x1) == 0;
					}
					cia2->TB_enable = val & 0x1;
					cia2->TB_input_mode = (val >> 0x5) & 0x1;
					break;
			}
		}
	}

	if (kernal_rom_enabled && addr >= KERNAL_ROM_START) return; /* KERNAL */
	if (basic_rom_enabled && addr >= BASIC_ROM_START && addr <= BASIC_ROM_END) return; /* BASIC */
	//whatever if ((char_rom_enabled || io_enabled) && addr >= IO_CHAR_START && addr <= IO_CHAR_END) return; /* IO */

	bus->RAM[addr] = val;
}

// direct access of VIC to RAM
uint8_t	ram_dir_read(_bus* bus, uint16_t addr) {
	return bus->RAM[addr];
}

void	ram_dir_write(_bus* bus, uint16_t addr, uint8_t val) {
	bus->RAM[addr] = val;
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

	memcpy(bus->RAM + LOW_CHAR_ROM_START, buffer, CHAR_ROM_SIZE);
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
	bus->ram_read = ram_dir_read;
	bus->ram_write = ram_dir_write;
	bus->load_roms = load_roms;
}

