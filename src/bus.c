#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	if (addr == RASTER)
		return ((_VIC_II*)bus->ppu)->raster & 0xFF;
	else if (addr == CNTRL1) {
		if ((((_VIC_II*)bus->ppu)->raster >> 0x7) & 0x1)
			return bus->RAM[addr] | 0x1;
		return bus->RAM[addr] & ~0x1;
	}
	return bus->RAM[addr];
}

void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	if (addr == MEM_SETUP||addr == CNTRL1||addr == CNTRL2)
		printf("write: %04X == %02X\n", addr, val);
	bus->RAM[addr] = val;
}

uint8_t	ppu_read_(_bus *bus, uint16_t addr) {
	_VIC_II	*ppu = (_VIC_II*)bus->ppu;
	////
	(void)ppu;
	(void)addr;
	return 0;
}

void	ppu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_VIC_II	*ppu = (_VIC_II*)bus->ppu;
	////
	(void)ppu;
	(void)addr;
	(void)val;
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

