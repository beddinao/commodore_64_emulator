#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	return bus->RAM[addr];
}

void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
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

uint8_t	load_ROM(_bus *bus, char *filename) {
	(void)bus;
	char buffer[BASIC_ROM_SIZE];
	unsigned chars_read;

	FILE *file = fopen(filename, "rb");
	if (!file)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, BASIC_ROM_SIZE, file);
	printf("rom: %s\n", buffer);

	fclose(file);
	return 1;
}

void	bus_init(_bus *bus) {
	memset(bus->RAM, 0, sizeof(bus->RAM));
	bus->cpu_read = cpu_read_;
	bus->cpu_write = cpu_write_;
	bus->ppu_read = ppu_read_;
	bus->ppu_write = ppu_write_;
	bus->load_ROM = load_ROM;
}

