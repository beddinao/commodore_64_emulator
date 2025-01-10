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
	bus->RAM[0x2B] = 0x01;  // Start of BASIC text
	bus->RAM[0x2C] = 0x08;
	bus->RAM[0x2D] = 0x03;  // Start of variables
	bus->RAM[0x2E] = 0x08;
	// Screen control
	bus->RAM[0xD011] = 0x1B;  // VIC control register (enable screen, 25 rows)
	bus->RAM[0xD016] = 0xC8;  // VIC control register (40 columns)
	bus->RAM[0xD018] = 0x14;  // Default memory setup for screen and charset
	bus->RAM[0x0288] = 0x04;  // Current screen page
	// Keyboard/cursor control
	bus->RAM[0xC6] = 0x00;    // Clear keyboard buffer length
	bus->RAM[0xCF] = 0x00;    // Cursor blink flag
	bus->RAM[0xD0] = 0x00;    // Cursor blink phase
	bus->RAM[0xD3] = 0x00;    // Cursor column
	bus->RAM[0xD6] = 0x00;    // Current screen line
	// Color memory
	bus->RAM[0x0286] = 0x0E;  // Current character color (light blue)
	bus->RAM[0xD020] = 0x0E;  // Border color
	bus->RAM[0xD021] = 0x06;  // Background color
	// BASIC/KERNAL flags
	bus->RAM[0x9D] = 0x00;    // BASIC direct/program mode flag
	bus->RAM[0x98] = 0x00;    // BASIC end-of-line flag
	//
	((_6502*)bus->cpu)->PC = 0xE394;
	((_6502*)bus->cpu)->set_flag((_6502*)bus->cpu, 'D', 0);
	//
	_CIA *cia1 = (_CIA*)bus->cia1;
	_keymap *keys = (_keymap*)cia1->keys;

	keys->active_row = 0xFF;
	memset(keys->matrix, 0xFF, sizeof(keys->matrix));
	printf(":freed $%04X(%u) Bytes of memory\n", prg->size, prg->size);
	free(prg);
	bus->prg = NULL;
}
