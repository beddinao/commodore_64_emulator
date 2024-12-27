#include "metallc64.h"

uint8_t	cpu_read_(_bus *bus, uint16_t addr) {
	if (addr < 0x2000)
		return bus->ram[addr & 0x07FF];
	else if (addr < 0x4000)
		return bus->ppu_read(bus, 0x2000 + (addr & 0x7));
	else if (addr >= PRG_ROM)
		return bus->cartridge[addr - RAM_SIZE];
	return 0;
}

void	cpu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	if (addr < 0x2000)
		bus->ram[addr & 0x07FF] = val;
	else if (addr < 0x4000)
		bus->ppu_write(bus, 0x2000 + (addr & 0x7), val);
	else if (addr >= PRG_ROM)
		bus->cartridge[addr - RAM_SIZE] = val;
}

uint8_t	ppu_read_(_bus *bus, uint16_t addr) {
	_ppu	*ppu = (_ppu*)bus->ppu;
	uint8_t 	status;
	////
	switch (addr) {
		case 0x2000: return ppu->ctrl;
		case 0x2001: return ppu->mask;
		case 0x2002:
			status = ppu->status;
			ppu->status &= ~0x80;
			return status;
		case 0x2003: return ppu->oamaddr;
		case 0x2004: return ppu->oamdata;
		case 0x2005: return ppu->scroll;
		case 0x2006: return ppu->addr;
		case 0x2007: return ppu->data;
	}
	return 0;
}

void	ppu_write_(_bus *bus, uint16_t addr, uint8_t val) {
	_ppu	*ppu = (_ppu*)bus->ppu;
	////
	switch (addr) {
		case 0x2000: ppu->ctrl = val; break;
		case 0x2001: ppu->mask = val; break;
		case 0x2002: break;
		case 0x2003: ppu->oamaddr = val; break;
		case 0x2004: ppu->oamdata = val; break;
		case 0x2005: ppu->scroll = val; break;
		case 0x2006: ppu->addr = val; break;
		case 0x2007: ppu->data = val; break;
	}
}

uint8_t	load_ROM(_bus *bus, char *filename) {
	char buffer[PRGM_MSIZE];
	unsigned program_size, chr_rom;
	unsigned chars_read;
	FILE *file = fopen(filename, "rb");
	if (!file)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, 16, file);
	if (!chars_read || strncmp(buffer, "NES\x1A", 4)) {
		fclose(file);
		return 0;
	}
	
	bus->rstv = buffer[4] == 1 ? _16RSTV : _32RSTV;
	program_size = buffer[4] * _16KB;
	chr_rom = buffer[5] * _08KB;

	printf("\nprogram_size: %04X(%u) [%04X(%04X) -> %04X(%04X)], chr_rom: %04X(%u)\n",
		program_size, program_size,
		PRG_ROM, PRG_ROM + RAM_SIZE,
		PRG_ROM + program_size, PRG_ROM + program_size + RAM_SIZE,
		chr_rom, chr_rom);

	if (!program_size || program_size > PRGM_MSIZE) {
		fclose(file);
		return 0;
	}

	printf("\n");
	for (unsigned ram_addr = 0; ram_addr < chars_read; ram_addr++) {
		printf("%X: %X", ram_addr, buffer[ ram_addr ]);
		if (ram_addr < 3)	printf(" == %c", buffer[ram_addr]);
		printf("\n");
	}
	printf("\n");

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, program_size, file);
	if (!chars_read || chars_read != program_size) {
		fclose(file);
		return 0;
	}
	memcpy(bus->cartridge + PRG_ROM, buffer, program_size);
	if (bus->rstv == _16RSTV) {
		for (unsigned c_addr = program_size; c_addr < PRGM_MSIZE; c_addr += program_size)
			memcpy(bus->cartridge + PRG_ROM + c_addr, buffer, program_size);
	}

	printf("chars_read: %04X(%u), File reset vector: %02X %02X\n",
		chars_read, chars_read,
		buffer[program_size - 2], buffer[program_size - 1]);

	if (chr_rom && chr_rom <= NME_TAB) {
		memset(buffer, 0, sizeof(buffer));
		chars_read = fread(buffer, 1, chr_rom, file);
		if (!chars_read || chars_read != chr_rom) {
			fclose(file);
			return 0;
		}
		memcpy(bus->vram, buffer, chr_rom);
		/*printf("\nPATTERN TABLE:\n");
		for (unsigned ram_addr = 0, i = 0; ram_addr < chr_rom; ram_addr += 16, i++) {
			printf("%03u: 0x%04X(%04u): ",i, ram_addr, ram_addr);
			for (unsigned col = 0; col < 16 && col + ram_addr < chr_rom; col++)
				printf("%02X ", bus->ppu_read(bus, col + ram_addr));
			printf("\n");
		}
		printf("\n");*/
	}
	else	memset(bus->chr_ram, 0, sizeof(bus->chr_ram));

	fclose(file);
	return 1;
}

void	bus_init(_bus *bus) {
	memset(bus->ram, 0, sizeof(bus->ram));
	memset(bus->vram, 0, sizeof(bus->vram));
	memset(bus->cartridge, 0, sizeof(bus->cartridge));
	bus->cpu_read = cpu_read_;
	bus->cpu_write = cpu_write_;
	bus->ppu_read = ppu_read_;
	bus->ppu_write = ppu_write_;
	bus->load_ROM = load_ROM;
}

