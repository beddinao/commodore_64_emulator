#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	switch (addr) {
		case RASTER:
			return ((_VIC_II*)bus->ppu)->raster & 0xFF;
		case CNTRL1:
			if ((((_VIC_II*)bus->ppu)->raster >> 0x7) & 0x1)
				return bus->RAM[addr] | 0x1;
			return bus->RAM[addr] & ~0x1;

		case (0xDC << 0x8 | TIMERA_LOW):	return ((_CIA*)bus->cia1)->timerA & 0xFF;
		case (0xDC << 0x8 | TIMERA_HIGH):	return (((_CIA*)bus->cia1)->timerA >> 0x8) & 0xFF;
		case (0xDC << 0x8 | TIMERB_LOW):	return ((_CIA*)bus->cia1)->timerB & 0xFF;
		case (0xDC << 0x8 | TIMERB_HIGH):	return (((_CIA*)bus->cia1)->timerB >> 0x8) & 0xFF;

		case (0xDD << 0x8 | TIMERA_LOW):	return ((_CIA*)bus->cia2)->timerA & 0xFF;
		case (0xDD << 0x8 | TIMERA_HIGH):	return (((_CIA*)bus->cia2)->timerA >> 0x8) & 0xFF;
		case (0xDD << 0x8 | TIMERB_LOW):	return ((_CIA*)bus->cia2)->timerB & 0xFF;
		case (0xDD << 0x8 | TIMERB_HIGH):	return (((_CIA*)bus->cia2)->timerB >> 0x8) & 0xFF;

		default:	return bus->RAM[addr];
	}
}

void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_VIC_II *ppu = (_VIC_II*)bus->ppu;

	switch (addr) {
		case MEM_SETUP: // D018
			if ((bus->cpu_read(bus, CNTRL1) >> 0x5) & 0x1)
				ppu->bitmap_ram = (val & 0x8) ? 0x2000 : 0x0000;
			switch ((val & 0xF) >> 0x1) {
				case 0x0:	ppu->char_ram = 0x0; break;
				case 0x1:	ppu->char_ram = 0x0800; break;
				case 0x2:	ppu->char_ram = 0x1000; break;
				case 0x3:	ppu->char_ram = 0x1800; break;
				case 0x4:	ppu->char_ram = 0x2000; break;
				case 0x5:	ppu->char_ram = 0x2800; break;
				case 0x6:	ppu->char_ram = 0x3000; break;
				case 0x7:	ppu->char_ram = 0x3800; break;
				default:	return;
			}
			switch (val >> 0x4) {
				case 0x0:	ppu->screen_ram = 0x0; break;
				case 0x1:	ppu->screen_ram = 0x0400; break;
				case 0x2:	ppu->screen_ram = 0x0800; break;
				case 0x3:	ppu->screen_ram = 0x0C00; break;
				case 0x4:	ppu->screen_ram = 0x1000; break;
				case 0x5:	ppu->screen_ram = 0x1400; break;
				case 0x6:	ppu->screen_ram = 0x1800; break;
				case 0x7:	ppu->screen_ram = 0x1C00; break;
				case 0x8:	ppu->screen_ram = 0x2000; break;
				case 0x9:	ppu->screen_ram = 0x2400; break;
				case 0xA:	ppu->screen_ram = 0x2800; break;
				case 0xB:	ppu->screen_ram = 0x2C00; break;
				case 0xC:	ppu->screen_ram = 0x3000; break;
				case 0xD:	ppu->screen_ram = 0x3400; break;
				case 0xE:	ppu->screen_ram = 0x3800; break;
				case 0xF:	ppu->screen_ram = 0x3C00; break;
				default:	return;
			}
			break;
		case CIA2_START: // DD00
			switch (val & 0x3) {
				case 0x0:	ppu->bank = VIC_BANK_3; break;
				case 0x1:	ppu->bank = VIC_BANK_2; break;
				case 0x2:	ppu->bank = VIC_BANK_1; break;
				case 0x3:	ppu->bank = VIC_BANK_0; break;
				default:	break;
			}
			break;
		case (0xDC << 0x8 | TIMERA_LOW):	((_CIA*)bus->cia1)->latchA_low = val; break;
		case (0xDC << 0x8 | TIMERB_LOW):	((_CIA*)bus->cia1)->latchB_low = val; break;
		case (0xDD << 0x8 | TIMERA_LOW):	((_CIA*)bus->cia2)->latchA_low = val; break;
		case (0xDD << 0x8 | TIMERB_LOW):	((_CIA*)bus->cia2)->latchB_low = val; break;
		case (0xDC << 0x8 | TIMERA_HIGH):
					((_CIA*)bus->cia1)->latchA_high = val;
					if (!((_CIA*)bus->cia1)->timerA)
						((_CIA*)bus->cia1)->timerA = ((_CIA*)bus->cia1)->latchA_high << 0x8 |
							((_CIA*)bus->cia1)->latchA_low;
					break;
		case (0xDC << 0x8 | TIMERB_HIGH):
					((_CIA*)bus->cia1)->latchB_high = val;
					if (!((_CIA*)bus->cia1)->timerB)
						((_CIA*)bus->cia1)->timerB = ((_CIA*)bus->cia1)->latchB_high << 0x8 |
							((_CIA*)bus->cia1)->latchB_low;
					break;
		case (0xDD << 0x8 | TIMERA_HIGH):
					((_CIA*)bus->cia2)->latchA_high = val;
					if (!((_CIA*)bus->cia2)->timerA)
						((_CIA*)bus->cia2)->timerA = ((_CIA*)bus->cia2)->latchA_high << 0x8 |
							((_CIA*)bus->cia2)->latchA_low;
					break;
		case (0xDD << 0x8 | TIMERB_HIGH):
					((_CIA*)bus->cia2)->latchB_high = val;
					if (!((_CIA*)bus->cia2)->timerB)
						((_CIA*)bus->cia2)->timerB = ((_CIA*)bus->cia2)->latchB_high << 0x8 |
							((_CIA*)bus->cia2)->latchB_low;
					break;
		case (0xDC << 0x8 | TIMERA_CNTRL): ((_CIA*)bus->cia1)->timerA_ctrl = val; break;
		case (0xDC << 0x8 | TIMERB_CNTRL): ((_CIA*)bus->cia1)->timerB_ctrl = val; break;
		case (0xDD << 0x8 | TIMERA_CNTRL): ((_CIA*)bus->cia2)->timerA_ctrl = val; break;
		case (0xDD << 0x8 | TIMERB_CNTRL): ((_CIA*)bus->cia2)->timerB_ctrl = val; break;
		case (0xDC << 0x8 | CIA_CNTRL): ((_CIA*)bus->cia1)->intr_ctrl = val; break;
		case (0xDD << 0x8 | CIA_CNTRL): ((_CIA*)bus->cia2)->intr_ctrl = val; break;
		default: break;
	}
	bus->RAM[addr] = val;
}

uint8_t	ppu_read_(_bus *bus, uint16_t addr) {
	_VIC_II	*ppu = (_VIC_II*)bus->ppu;
	return bus->RAM[ppu->bank + addr];
}

void	ppu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_VIC_II	*ppu = (_VIC_II*)bus->ppu;
	bus->RAM[ppu->bank + addr] = val;
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

	memcpy(bus->RAM + UPP_CHAR_ROM_START, buffer, CHAR_ROM_SIZE);
	memcpy(bus->RAM + LOW_CHAR_ROM_START, buffer, CHAR_ROM_SIZE);
	fclose(file);
	return 1;
}

void	bus_init(_bus *bus) {
	memset(bus->RAM, 0, sizeof(bus->RAM));
	bus->cpu_read = cpu_read_;
	bus->cpu_write = cpu_write_;
	bus->ppu_read = ppu_read_;
	bus->ppu_write = ppu_write_;
	bus->load_kernal = load_kernal;
	bus->load_basic = load_basic;
	bus->load_chars = load_chars;
}

