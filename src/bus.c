#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	_CIA *cia1 = (_CIA*)bus->cia1;
	_CIA *cia2 = (_CIA*)bus->cia2;
	_cia_tod *tod1 = (_cia_tod*)cia1->TOD;
	_cia_tod *tod2 = (_cia_tod*)cia2->TOD;
	_VIC_II *vic = (_VIC_II*)bus->vic;
	_keymap *keys = (cia1 && cia1->keys) ? (_keymap*)cia1->keys : NULL;

	uint8_t retuv = 0;

	uint8_t proc_port = bus->RAM[1];
	bool basic_rom_enabled = (proc_port & 0x3) == 0x3; // $A000-$BFFF --xx
	bool kernal_rom_enabled = (proc_port & 0x2) == 0x2; // $E000-$FFFF --xx
	bool io_enabled = (proc_port & 0x4) == 0x4; // $D000-$DFFF -x--
	bool char_rom_enabled = (proc_port & 0x4) == 0x0; // $D000-$DFFF -x--

	/*  KERNAL */
	if (addr >= KERNAL_ROM_START && kernal_rom_enabled)
		return bus->KERNAL[addr - KERNAL_ROM_START];
	/*  BASIC  */
	if (addr >= BASIC_ROM_START && addr <= BASIC_ROM_END && basic_rom_enabled)
		return bus->BASIC[addr - BASIC_ROM_START];

	if (addr >= IO_CHAR_START && addr <= IO_CHAR_END && io_enabled) {
		/*  CHARACTERS  */
		if (char_rom_enabled)
			return bus->CHARACTERS[addr - IO_CHAR_START];
		/*  IO  */
		else if (io_enabled) {
			// VIC Registers
			if (addr >= VIC_REG_START && addr <= VIC_REG_END) {
				// Mirror $D000-$D3FF -> $D000-$D03F
				uint8_t low_nibble = addr & 0x3F;
				addr = ((addr >> 0x8) & 0xFF) << 0x8 | low_nibble;
				switch (addr) {
					case INTR_STATUS: // $D019
						if (vic->raster_interrupt_triggered) retuv |= 0x1;
						if (vic->sp_bg_interrupt_triggered) retuv |= 0x2;
						if (vic->sp_sp_interrupt_triggered) retuv |= 0x4;
						vic->raster_interrupt_triggered = FALSE;
						vic->sp_bg_interrupt_triggered = FALSE;
						vic->sp_sp_interrupt_triggered = FALSE;
						return retuv;
					case RASTER: // $D012
						return vic->raster & 0xFF;
					case CNTRL1: // $D011
						return ((vic->raster >> 0x7) & 0x1) ?
							(vic->control1 | 0x80):
							(vic->control1 & ~0x80);
					case CNTRL2: // $D016
						return vic->control2;
					case 0xD01E:
						retuv = vic->sp_sp_collision;
						return retuv;
					case 0xD01F:
						retuv = vic->sp_bg_collision;
						return retuv;
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
					case 0xDC04: /* Timer A low */  return cia1->timerA & 0xFF;
					case 0xDC05: /* Timer A high*/  return (cia1->timerA >> 0x8) & 0xFF;
					case 0xDC06: /* Timer B low */  return cia1->timerB & 0xFF;
					case 0xDC07: /* Timer B high*/  return (cia1->timerB >> 0x8) & 0xFF;
					case 0xDC08: /* TOD th secs*/
								  retuv = tod1->latched ? tod1->th_secs_latch : tod1->th_secs;
								  tod1->latched = FALSE;
								  return (retuv/10) << 4 | (retuv%10); 
								  // 123 Decimal -> (12 << 4 | 3) -> 11000011 195
					case 0xDC09: /* TOD secs */
								  retuv = tod1->latched ? tod1->secs_latch : tod1->secs;
								  return (retuv/10) << 4 | (retuv%10);
					case 0xDC0A: /* TOD mins */
								  retuv = tod1->latched ? tod1->mins_latch : tod1->mins;
								  return (retuv/10) << 4 | (retuv%10);
					case 0xDC0B: /* TOD hrs */
								  tod1->th_secs_latch = tod1->th_secs;
								  tod1->secs_latch = tod1->secs;
								  tod1->mins_latch = tod1->mins;
								  tod1->latched = TRUE;
								  retuv = (tod1->hrs/10) << 4 | (tod1->hrs%10);
								  return (tod1->PM) ?
									  (retuv | 0x80) :
									  (retuv & ~0x80);
					case 0xDC0C: return retuv;
					case 0xDC0D: /* Interrupt Control */
						   if (cia1->TA_interrupt_triggered || cia1->TB_interrupt_triggered
								   || tod1->interrupt_triggered) {
							   retuv |= 0x80;
							   if (cia1->TA_interrupt_triggered) retuv |= 0x1;
							   if (cia1->TB_interrupt_triggered) retuv |= 0x2;
							   if (tod1->interrupt_triggered) retuv |= 0x4;
							   cia1->TA_interrupt_triggered = FALSE;
							   cia1->TB_interrupt_triggered = FALSE;
							   tod1->interrupt_triggered = FALSE;
						   }
						   return retuv;
					case 0xDC0E: return retuv;
					case 0xDC0F: return retuv;
						   /*
						      CIA 2 $DD0x
						      */
					case 0xDD04: /* Timer A low */ return cia2->timerA & 0xFF;
					case 0xDD05: /* Timer A high */ return (cia2->timerA >> 0x8) & 0xFF;
					case 0xDD06: /* Timer B low */ return cia2->timerB & 0xFF;
					case 0xDD07: /* Timer B high */ return (cia2->timerB >> 0x8) & 0xFF;
					case 0xDD08: /* TOD th secs*/
								  retuv = tod2->latched ? tod2->th_secs_latch : tod2->th_secs;
								  tod2->latched = FALSE;
								  return (retuv/10) << 4 | (retuv%10);
					case 0xDD09: /* TOD secs */
								  retuv = tod2->latched ? tod2->secs_latch : tod2->secs;
								  return (retuv/10) << 4 | (retuv%10);
					case 0xDD0A: /* TOD mins */
								  retuv = tod2->latched ? tod2->mins_latch : tod2->mins;
								  return (retuv/10) << 4 | (retuv%10);
					case 0xDD0B: /* TOD hrs */
								  tod2->th_secs_latch = tod2->th_secs;
								  tod2->secs_latch = tod2->secs;
								  tod2->mins_latch = tod2->mins;
								  tod2->latched = TRUE;
								  retuv = (tod2->hrs/10) << 4 | (tod2->hrs%10);
								  return (tod2->PM) ?
									  (retuv | 0x80) :
									  (retuv & ~0x80);
					case 0xDD0C: return retuv;
					case 0xDD0D: /* Interrupt Control */
						   if (cia2->TA_interrupt_triggered || cia2->TB_interrupt_triggered
								   || tod2->interrupt_triggered) {
							   retuv |= 0x80;
							   if (cia2->TA_interrupt_triggered) retuv |= 0x1;
							   if (cia2->TB_interrupt_triggered) retuv |= 0x2;
							   if (tod2->interrupt_triggered) retuv |= 0x4;
							   cia2->TA_interrupt_triggered = FALSE;
							   cia2->TB_interrupt_triggered = FALSE;
							   tod2->interrupt_triggered = FALSE;
						   }
						   return retuv;
					case 0xDD0E: return retuv;
					case 0xDD0F: return retuv;
					default:
						   return bus->RAM[addr];
				}
			}
		}
	}
	return bus->RAM[addr];
}


void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_VIC_II *vic = (_VIC_II*)bus->vic;
	_CIA* cia1 = (_CIA*)bus->cia1;
	_CIA* cia2 = (_CIA*)bus->cia2;
	_cia_tod* tod1 = (_cia_tod*)cia1->TOD;
	_cia_tod* tod2 = (_cia_tod*)cia2->TOD;

	switch (addr) {
		// protected: BASIC TOP RAM / PRG max addr
		case 0x01: bus->RAM[addr] = val; return;
		case 0xD015: /* TODO:remove */ bus->RAM[addr] = 0x0; return;
		//case 0x37: bus->RAM[addr] = 0x00; return;
		//case 0x38: bus->RAM[addr] = 0xA0; return;
	}

	uint8_t proc_port = bus->RAM[1];
	bool io_enabled = (proc_port & 0x4) == 0x4; // $D000-$DFFF -x--

	/*  IO  */
	if (addr >= IO_CHAR_START && addr <= IO_CHAR_END && io_enabled) {
		// VIC Registers
		if (addr >= VIC_REG_START && addr <= VIC_REG_END) {
			// Mirror $D000-$D3FF -> $D000-$D03F
			uint8_t low_nibble = addr & 0x3F;
			addr = ((addr >> 0x8) & 0xFF) << 0x8 | low_nibble;
			if (addr <= 0xD00F) {
				if ((addr & 0xF) % 2)
					vic->sprite_y[(addr & 0xF) / 2] = val;
				else	vic->sprite_x[(addr & 0xF) / 2] = val;
			}
			else if (addr >= 0xD027 && addr <= 0xD02E)
				vic->sprite_colors[(addr & 0xFF)-0x27] = val & 0xF;
			else {
				switch (addr) {
					case 0xD010:
						vic->sprite_8x = val;
						break;
					case CNTRL1: // $D011
						vic->control1 = val;
						break;
					case 0xD015:
						vic->sprite_enable = val;
						break;
					case CNTRL2: // $D016
						vic->control2 = val;
						break;
					case 0xD017:
						vic->sprite_expand_y = val;
						break;
					case MEM_SETUP: // $D018
						vic->screen_ram = (val & 0xf0) << 6;
						if ((vic->control1 >> 0x5) & 0x1) 
							vic->bitmap_ram = (val & 0x8) << 10;
						else {
							uint8_t bva = (val >> 0x1) & 0x7;
							vic->char_rom_on = FALSE;
							if ((bva == 0x3 || bva == 0x2)
								&& (vic->bank == VIC_BANK_0 || vic->bank == VIC_BANK_2))
								vic->char_rom_on = TRUE;
							vic->char_ram = (val & 0xe) << 10;
						}
						bus->RAM[addr] |= 0x1;
						break;
					case 0xD01A:
						vic->raster_interrupt_enable = val & 0x1;
						vic->sp_bg_interrupt_enable = (val >> 0x1) & 0x1;
						vic->sp_sp_interrupt_enable = (val >> 0x2) & 0x1;
						break;
					case 0xD01B:
						vic->sprite_priority = val;
						break;
					case 0xD01C:
						vic->sprite_multicolor_enable = val;
						break;
					case 0xD01D:
						vic->sprite_expand_x = val;
						break;
					case 0xD025:
						vic->sprite_multicolor0 = val & 0xF;
						break;
					case 0xD026:
						vic->sprite_multicolor1 = val & 0xF;
						break;
				}
			}
		}
		// CIAs Registers
		else if ((addr >= CIA1_START && addr <= CIA1_END)
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
				case 0xDC04: // Timer A low
					cia1->TA_latch_low = val;
					break;
				case 0xDC05: // Timer A high
					cia1->TA_latch_high = val;
					break;
				case 0xDC06: // Timer B low
					cia1->TB_latch_low = val;
					break;
				case 0xDC07: // Timer B high
					cia1->TB_latch_high = val;
					break;
				case 0xDC08: // TOD tenth seconds
					if (tod1->write_mode)
						tod1->th_secs_alarm = val;
					else 	tod1->th_secs = val;
					break;
				case 0xDC09: // TOD seconds
					if (tod1->write_mode)
						tod1->secs_alarm = val;
					else 	tod1->secs = val;
					break;
				case 0xDC0A: // TOD minutes
					if (tod1->write_mode)
						tod1->mins_alarm = val;
					else 	tod1->mins = val;
					break;
				case 0xDC0B: // TOD hours
					if (tod1->write_mode)
						tod1->hrs_alarm = val;
					else 	tod1->hrs = val;
					break;
				case 0xDC0D: // Interrupt control
					if (val & 0x1) cia1->TA_interrupt_enable = (val >> 0x7) & 0x1;
					if ((val >> 0x1) & 0x1) cia1->TB_interrupt_enable = (val >> 0x7) & 0x1;
					if ((val >> 0x2) & 0x1) tod1->interrupt_enable = (val >> 0x7) & 0x1;
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
					tod1->write_mode = (val >> 0x7) & 0x1;
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
				case 0xDD08: // TOD tenth seconds
					if (tod2->write_mode)
						tod2->th_secs_alarm = val;
					else	tod2->th_secs = val;
					break;
				case 0xDD09: // TOD seconds
					if (tod2->write_mode)
						tod2->secs_alarm = val;
					else	tod2->secs = val;
					break;
				case 0xDD0A: // TOD minutes
					if (tod2->write_mode)
						tod2->mins_alarm = val;
					else	tod2->mins = val;
					break;
				case 0xDD0B: // TOD hours
					if (tod2->write_mode)
						tod2->hrs_alarm = val;
					else	tod2->hrs = val;
					break;
				case 0xDD0D: // Interrupt control 
					if (val & 0x1) cia2->TA_interrupt_enable = (val >> 0x7) & 0x1;
					if ((val >> 0x1) & 0x1) cia2->TB_interrupt_enable = (val >> 0x7) & 0x1;
					if ((val >> 0x2) & 0x1) tod2->interrupt_enable = (val >> 0x7) & 0x1;
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
					tod2->write_mode = (val >> 0x7) & 0x1;
					break;
			}
		}
	}
	bus->RAM[addr] = val;
}

// direct access to RAM(ignore $0001)
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

	memcpy(bus->BASIC, buffer, BASIC_ROM_SIZE);
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

	memcpy(bus->KERNAL, buffer, KERNAL_ROM_SIZE);
	fclose(file);

	unsigned nmi_ker_addr = NMI - KERNAL_ROM_START;
	unsigned rstv_ker_addr = RSTV - KERNAL_ROM_START;
	unsigned irq_ker_addr = IRQ_BRK - KERNAL_ROM_START;
	printf("\n\
%s       __  __      _        _ _  _____  __ _  _%s\n\
%s      |  \\/  |    | |      | | |/ ____|/ /| || |%s\n\
%s      | \\  / | ___| |_ __ _| | | |    / /_| || |_%s\n\
%s      | |\\/| |/ _ \\ __/ _` | | | |   | '_ \\__   _|%s\n\
%s      | |  | |  __/ || (_| | | | |___| (_) | | |%s\n\
%s      |_|  |_|\\___|\\__\\__,_|_|_|\\_____\\___/  |_|%s\n\
      the metall Commodore 64 emulator\n\
 %s\n\
      - Kernal interrupt vectors:\n\
        NMI: $%04x, RST: $%04x, IRQ: $%04x\n\
      - Window dimensions:\n\
        504x312 (PAL-display) -> %ux%u\n\
      - HLP/HELP/help for a list of commands\n\
      - NOTE: Not very accurate emulator\n\
 %s\n",
	WHT, RST, WHT, RST, WHT, RST,
 	"\e[0;97m", RST, "\e[0;97m", RST, "\e[1;97m", RST,
	WHT, WWIDTH, WHEIGHT,
 	bus->KERNAL[nmi_ker_addr + 1] << 0x8 | bus->KERNAL[nmi_ker_addr],
	bus->KERNAL[rstv_ker_addr + 1] << 0x8 | bus->KERNAL[rstv_ker_addr],
	bus->KERNAL[irq_ker_addr + 1] << 0x8 | bus->KERNAL[irq_ker_addr], RST);
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

	memcpy(bus->CHARACTERS, buffer, CHAR_ROM_SIZE);
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

static	void	bus_clean(_bus *bus) {
	free(bus->vic);
	free(bus->cpu);
	((_CIA*)bus->cia1)->clean(bus->cia1);
	((_CIA*)bus->cia2)->clean(bus->cia2);
	free(bus->cia1);
	free(bus->cia2);
}

_bus	*bus_init() {
	_bus *bus = malloc(sizeof(_bus));
	if (!bus) return FALSE;
	memset(bus, 0, sizeof(_bus));
	bus->reset = bus_init;
	bus->cpu_read = cpu_read_;
	bus->cpu_write = cpu_write_;
	bus->ram_read = ram_dir_read;
	bus->ram_write = ram_dir_write;
	bus->load_roms = load_roms;
	bus->clean = bus_clean;
	if (!bus->load_roms(bus)) {
		free(bus);
		return FALSE;
	}
	bus->RAM[1] = 0x37;
	return bus;
}

