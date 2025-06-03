#include <c64_emu.h> 

void	change_col(_bus* bus, _cmd *cmd) {
	cmd->col -= 1;
	if (!strcmp(cmd->cmd, "BRD"))
		bus->RAM[BRD_COLOR] = cmd->col;
	else if (!strcmp(cmd->cmd, "BGR"))
		bus->RAM[BACKG_COLOR0] = cmd->col;
	else	bus->RAM[0x0286] = cmd->col;
	free(cmd);
	bus->cmd = NULL;
}

void	prg_load_sequence(_bus *bus, _prg* prg) {
	/* load to RAM */
	memset(bus->RAM + BASIC_PRG_START, 0, prg->size);
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
	printf("\t:%sloaded%s program successfully at $%04x\n", GRN, RST, BASIC_PRG_START);
	printf("\t!Use the kernal %sRUN%s command to execute\n", UND, RST);
	printf("\t!Use the Shell %sCLR%s command to free\n", UND, RST);
}

void	reset_prg(_bus *bus, _prg* prg) {
	printf("\t- clearing program from memory..\n");
	/* hard reset */
	memset(bus->RAM, 0, sizeof(bus->RAM));
	bus->RAM[1] = 0x37;
	free(bus->cpu);
	bus->cpu = cpu_init(bus);
	if (!bus->cpu) {
		printf("%s:error%s: hard reset failed exiting..\n\n", RED, RST);
		exit(1);
	}
	bus->prg = NULL;
	printf("\t:%sfreed%s $%04x(%u) Bytes of memory\n", GRN, RST, prg->size, prg->size);
	free(prg);
}
