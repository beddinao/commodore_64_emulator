#include "metallc64.h"

void	print_byte(uint8_t byte) {
	for (unsigned i = 0; i < 0x8; i++)
		printf("%u ", (byte >> i) & 0x1);
}

void	dump_mem_area(_bus *bus, uint16_t st, uint16_t en) {
	for (unsigned row_addr = st; row_addr <= en; row_addr += 0x10) {
		printf("$%04X: ", row_addr);
		for (unsigned col = 0; col < 0x10 && col + row_addr < en; col++)
			printf("%02X ", bus->RAM[row_addr + col]);
		printf("\n");
	}
}

void	dump_regs(_bus *bus, uint16_t st, uint16_t en) {
	for (unsigned row_addr = st; row_addr <= en; row_addr++) {
		printf("$%04X: $%02X => ", row_addr, bus->RAM[row_addr]);
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
	printf("invalid syntax\"%s\"\n", line);
	printf("avilable commands:\n\n");
	printf("\tLDP $.prg : load BASIC program to memory\n");
	printf("\tLDD $.d64 : load D64 disk image(not-implemented)\n");
	printf("\tLDT $.T64 : load T64 tape image(not-implemented)\n\n");

	printf("\tBRD $col_i: change default border color\n");
	printf("\tBGR $col_i: change default background color\n");
	printf("\tTXT $col_i: change default text color(not-implemented)\n\n");

	printf("\tcmd $st $en : dump memory from address st to en(not-implemented)\n");
	printf("\tSCR : show CPU status\n");
	printf("\tSVR : show VIC-II registers\n");
	printf("\tSC1 : show CIA#1 registers\n");
	printf("\tSC2 : show CIA#2 registers\n\n");

	printf("\tCLR : exits/clear loaded program from memory\n");
	printf("\tHLP : show this help message\n");
	printf("\tEXT : exit emulation\n\n");
}

void	print_col_help(char *line) {
	printf("invalid syntax\"%s\"\n\n", line);
	printf("\tthe C64 have a palette of 16 colors:\n");
	printf("\tchose between 1 and 16 as indexes to that palette\n\n");
}

void	print_memory(_bus *bus, _cmd *cmd) {
	printf("\n");
	if (!strcmp(cmd->cmd, "SCR")) { // cpu registers
		_6502 *mos6502 = (_6502*)bus->cpu;
		printf("CPU registers:\n");
		printf("PC: $%04X\nA: $%02X, X: $%02X, Y: $%02X, SP: $%02X\n",
			mos6502->PC, mos6502->A, mos6502->X, mos6502->Y, mos6502->SP);
		printf("SR(P): ");
		print_byte(mos6502->SR);
		printf("\n");
		printf("last-opcode: $%02X\n", mos6502->opcode);
		printf("pending interrupts: NMI(%u), IRQ(%u)\n",
			mos6502->nmi_pending, mos6502->irq_pending);
	}
	else if (!strcmp(cmd->cmd, "SVR")) { // vic registers
		printf("VIC-II memory registers $%04X -> $%04X:\n", 0xD000, 0xD02E);
		dump_regs(bus, SPRITE0_X, SPR_CLR_7);
	}
	else if (!strcmp(cmd->cmd, "SC1")) { // cia#1 registers
		printf("CIA#1 memory registers $%04X -> $%04X:\n", 0xDC00, 0xDC0F);
		dump_regs(bus, 0xDC00, 0xDC0F);
	}
	else if (!strcmp(cmd->cmd, "SC2")) { // cia#2 registers
		printf("CIA#2 memory registers $%04X -> $%04X:\n", 0xDD00, 0xDD0F);
		dump_regs(bus, 0xDD00, 0xDD0F);
	}
	printf("\n");

	free(cmd);
	bus->cmd = NULL;
}
