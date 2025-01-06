#include "metallc64.h"

void	change_col(_bus* bus, _cmd *cmd) {
	cmd->col -= 1;
	if (!strcmp(cmd->cmd, "BRD"))
		bus->RAM[BRD_COLOR] = cmd->col;
	else if (!strcmp(cmd->cmd, "BGR"))
		bus->RAM[BACKG_COLOR0] = cmd->col;
	else	bus->RAM[BRD_COLOR] = cmd->col;
	free(cmd);
	bus->cmd = NULL;
}

void	prg_load_sequence(_bus *bus, _prg* prg) {
	/* clear region */
	memset(bus->RAM + BASIC_PRG_START, 0, prg->size);
	/* load to RAM */
	memcpy(bus->RAM + BASIC_PRG_START, prg->buffer, prg->size);
	/* ZERO-PAGE pointers setup */
	// leading zero byte
	bus->RAM[0x800] = 0x00;
	// $2B-$2C TXTTAB
	bus->RAM[0x2B] = 0x01;
	bus->RAM[0x2C] = 0x08;
	// $2D-$2E VARTAB
	bus->RAM[0x2D] = prg->en_addr & 0xFF;
	bus->RAM[0x2E] = (prg->en_addr >> 0x8) & 0xFF;
	// $2F-$30 ARYTAB
	bus->RAM[0x2F] = prg->en_addr & 0xFF;
	bus->RAM[0x30] = (prg->en_addr >> 0x8) & 0xFF;
	// $31-$32 STREND
	bus->RAM[0x31] = prg->en_addr & 0xFF;//0x00;
	bus->RAM[0x32] = (prg->en_addr >> 0x8) & 0xFF;//0xA0;
	// $33-$34 FRETOP
	bus->RAM[0x33] = prg->en_addr & 0xFF;
	bus->RAM[0x34] = (prg->en_addr >> 0x8) & 0xFF;
	prg->loaded = TRUE;
	printf("program loaded successfully at $%04X\n", BASIC_PRG_START);
}

void	reset_prg(_bus *bus, _prg* prg) {
	printf("clearing program from memory..\n");
	memset(bus->RAM + BASIC_PRG_START, 0, prg->size);
	bus->RAM[0x800] = 0x00;
	bus->RAM[0x801] = 0x00;
	bus->RAM[0x802] = 0x00;
	bus->RAM[0x803] = 0x00;
	bus->RAM[0x804] = 0x00;
	//
	bus->RAM[0x2B] = 0x05;
	bus->RAM[0x2C] = 0x08;
	//
	bus->RAM[0x2D] = 0x01;
	bus->RAM[0x2E] = 0x08;
	//
	bus->RAM[0x2F] = 0x05;
	bus->RAM[0x30] = 0x08;
	//
	bus->RAM[0x31] = 0x05;
	bus->RAM[0x32] = 0x08;
	//
	bus->RAM[0x33] = 0x00;
	bus->RAM[0x34] = 0xA0;
	//// Current BASIC line pointer
	bus->RAM[0x7A] = 0x01;
	bus->RAM[0x7B] = 0x08;
	//// Current BASIC line number
	bus->RAM[0xAE] = 0x00;
	bus->RAM[0xAF] = 0x00;
	//
	((_6502*)bus->cpu)->PC = 0xE394;
	((_6502*)bus->cpu)->set_flag((_6502*)bus->cpu, 'D', 0);
	printf(":freed $%04X(%u) Bytes of memory\n",
		prg->size, prg->size);
	free(prg);
	bus->prg = NULL;
}
