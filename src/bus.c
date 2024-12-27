#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	return bus->ram[addr];
}

void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	bus->ram[addr] = val;
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
	char buffer[RAM_SIZE];
	unsigned chars_read;

	FILE *file = fopen(filename, "rb");
	if (!file)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, RAM_SIZE, file);
	printf("rom: %s\n", buffer);

	fclose(file);
	return 1;
}

void	bus_init(_bus *bus) {
	memset(bus->ram, 0, sizeof(bus->ram));
	bus->cpu_read = cpu_read_;
	bus->cpu_write = cpu_write_;
	bus->ppu_read = ppu_read_;
	bus->ppu_write = ppu_write_;
	bus->load_ROM = load_ROM;
}

