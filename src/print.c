#include <c64_emu.h> 

void	print_byte(uint8_t byte) {
	uint8_t b;
	for (unsigned i = 0; i < 0x8; i++) {
		b = (byte >> i) & 0x1;
		printf("%s%u%s ", b ? UND : WHT, b, RST);
	}
}

void	dump_mem_area(_bus *bus, uint16_t st, uint16_t en) {
	uint8_t byte;
	for (unsigned row_addr = st; row_addr <= en; row_addr += 0x10) {
		printf("\t%s$%04x:%s ", WHT, row_addr, RST);
		for (unsigned col = 0; col < 0x10 && col + row_addr < en; col++) {
			byte = bus->RAM[row_addr + col];
			printf("%s%02x%s ", byte ? RST : WHT, byte, RST);
		}
		printf("\n");
	}
}

void	dump_regs(_bus *bus, uint16_t st, uint16_t en) {
	for (unsigned row_addr = st; row_addr <= en; row_addr++) {
		printf("\t%s$%04x:%s $%02x  ", WHT, row_addr, RST, bus->RAM[row_addr]);
		print_byte(bus->RAM[row_addr]);
		if (row_addr == 0xD011) 
			printf(",bitmap#5(%u), extended_bg#6(%u)",
				(bus->RAM[row_addr] >> 5) & 1,
				(bus->RAM[row_addr] >> 6) & 1);
		else if (row_addr == 0xD016)
			printf(",multicolor#4(%u)", (bus->RAM[row_addr] >> 4) & 1);
		printf("\n");
	}
}

void	print_help(char *line) {
	printf("%sinvalid syntax%s\"%s\"\n", RED, RST, line);
	printf("avilable commands:\n\n");
	printf("\tLDP $.prg : load BASIC program to memory\n");
	printf("\tLDD $.d64 : load D64 disk image\n");
	printf("\tLDT $.T64 : load T64 tape image(not-implemented)\n\n");

	printf("\tBRD $col_i: change default border color\n");
	printf("\tBGR $col_i: change default background color\n");
	printf("\tTXT $col_i: change default text color\n\n");

	printf("\tDMP $st $en : dump memory from address $st to $en\n");
	printf("\tSCR : show CPU status\n");
	printf("\tSVR : show VIC-II registers\n");
	printf("\tSC1 : show CIA#1 registers\n");
	printf("\tSC2 : show CIA#2 registers\n\n");

	printf("\tCLR : exits/clear loaded program from memory\n");
	printf("\tHLP/help : show this help message\n");
	printf("\tEXT : exit emulation\n\n");
}

void	print_col_help(char *line) {
	printf("%sinvalid syntax%s\"%s\"\n\n", RED, RST, line);
	printf("\tthe C64 have a palette of 16 colors:\n");
	printf("\tchose between 1 and 16 as indexes to that palette\n\n");
}

extern thread_data *t_data;

void	exec_ram_dump(char *cmd) {
	_bus* bus = (_bus*)t_data->bus;
	printf("\n");
	if (!strcmp(cmd, "CPU")) { // cpu registers
		_6502 *mos6502 = (_6502*)bus->cpu;
		printf("%sCPU registers%s:\n", CYN, RST);
		printf("\tSR(P): ");
		print_byte(mos6502->SR);
		printf("\n\t       C Z I D B - V N\n");
		printf("\tPC: $%02x\n\tA: $%02x\n\tX: $%02x, Y: $%02x\n\tSP: $%02x\n",
			mos6502->PC, mos6502->A, mos6502->X, mos6502->Y, mos6502->SP);
		printf("\tlast opcode: $%02x\n", mos6502->opcode);
		printf("\tpending interrupts: NMI(%u), IRQ(%u)\n",
			mos6502->nmi_pending, mos6502->irq_pending);
	}
	else if (!strcmp(cmd, "VIC-II")) { // vic registers
		printf("%sVIC-II memory registers%s $%02x -> $%02x:\n", CYN, RST, 0xD000, 0xD02E);
		dump_regs(bus, SPRITE0_X, SPR_CLR_7);
	}
	else if (!strcmp(cmd, "CIA-1")) { // cia#1 registers
		printf("%sCIA#1 memory registers%s $%02x -> $%02x:\n", CYN, RST, 0xDC00, 0xDC0F);
		dump_regs(bus, 0xDC00, 0xDC0F);
	}
	else if (!strcmp(cmd, "CIA-2")) { // cia#2 registers
		printf("%sCIA#2 memory registers%s $%02x -> $%02x:\n", CYN, RST, 0xDD00, 0xDD0F);
		dump_regs(bus, 0xDD00, 0xDD0F);
	}
	else printf("FUCK OFF");
	/*else if (!strcmp(cmd, "DMP")) { // range memory dump
		printf("%smemory from%s $%02x -> $%02x:\n", CYN, RST, cmd->st_addr, cmd->en_addr);
		dump_mem_area(bus, cmd->st_addr, cmd->en_addr);
	}*/
	printf("\n");

	/*free(cmd);
	bus->cmd = NULL;*/
}
