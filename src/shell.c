#include "metallc64.h"

extern thread_data	*t_data;

bool	exec_dmp(_bus *bus, char *cmd) {
	unsigned size = strlen(cmd);
	char *cmd_end = strstr(cmd, " ");
	char *addr_1 = cmd_end, *addr_2;
	unsigned i, addr1_size, addr2_size;
	
	/* some fine parsing */
	i = cmd_end - cmd;
	for (; i < size && *addr_1 == ' '; i++, addr_1++);
	if (size - i < 1) return FALSE;
	addr_2 = strstr(addr_1+1, " ");
	i = addr_2 - cmd;
	for (; i < size && *addr_2 == ' '; i++, addr_2++);
	if (size - i < 1) return FALSE;
	addr1_size = addr_2 - addr_1;
	if (!addr1_size || addr1_size > 5) return FALSE;
	addr2_size = &cmd[size] - addr_2;
	if (!addr2_size || addr2_size > 4) return FALSE;

	unsigned st = strtol(addr_1, NULL, 16);
	unsigned en = strtol(addr_2, NULL, 16);

	if (st >= en || en > 0xFFFF)
		return FALSE;
	
	_cmd *t_cmd = malloc(sizeof(_cmd));
	if (!t_cmd) return FALSE;
	memset(t_cmd, 0, sizeof(_cmd));
	t_cmd->st_addr = st;
	t_cmd->en_addr = en;
	memcpy(t_cmd->cmd, cmd, 3);
	bus->cmd = t_cmd;

	pthread_mutex_lock(&bus->t_data->cmd_mutex);
	bus->t_data->cmd = TRUE;
	pthread_mutex_unlock(&bus->t_data->cmd_mutex);
	usleep(100000);

	return TRUE;
}

void	exec_ldp(uint8_t *program, unsigned size) {
	_bus *bus = (_bus*)t_data->bus;
	size -= 2;

	uint16_t ld_addr = program[1] << 0x8 | program[0];
	if (!ld_addr || ld_addr != BASIC_PRG_START) {
		printf(":%serror:%s invalid BASIC program load address $%04x(.prg first two bytes):\n\
				must be inside BASIC prg area $%04x - $%04x\n\
				typically at $801\n",
				RED, RST, ld_addr, BASIC_PRG_START, BASIC_PRG_END);
		return;
	}

	if (!size || ld_addr + size > BASIC_PRG_END || ld_addr + size < BASIC_PRG_START) {
		printf(":%serror:%s invalid BASIC program size $%04x, += load-address($%04x) > $%04x:\n\
				must fit inside the specified BASIC prg area $%04x - $%04x\n",
				RED, RST, size, ld_addr, BASIC_PRG_END, BASIC_PRG_START, BASIC_PRG_END);
		return;
	}
	printf("\t- loading $%04x(%u) bytes at $%04x -> $%04x ...\n",
			size, size, ld_addr, ld_addr + size);

	_prg *prg = malloc(sizeof(_prg));
	if (!prg) {
		printf(":%serror:%s BASIC program failed to load to RAM\n", RED, RST);
		return;
	}
	memset(prg, 0, sizeof(_prg));
	prg->size = size;
	prg->ld_addr = BASIC_PRG_START;
	prg->en_addr = BASIC_PRG_START + size + 1;
	memcpy(prg->buffer, program + 2, size); // <-----------------
	bus->prg = prg;

	bus->t_data->load = TRUE;
	// prg_load_sequence(bus, prg);
	//
	//
	/*uint16_t en_addr = BASIC_PRG_START + size + 1;
	memset(bus->RAM + BASIC_PRG_START, 0, size);
	memcpy(bus->RAM + BASIC_PRG_START, program + 2, size);
	bus->RAM[0x800] = 0x00;
	// $2B-$2C TXTTAB
	bus->RAM[0x2B] = 0x01;
	bus->RAM[0x2C] = 0x08;
	// $2D-$2E VARTAB
	bus->RAM[0x2D] = en_addr & 0xFF;
	bus->RAM[0x2E] = (en_addr >> 0x8) & 0xFF;
	// $2F-$30 ARYTAB
	bus->RAM[0x2F] = en_addr & 0xFF;
	bus->RAM[0x30] = (en_addr >> 0x8) & 0xFF;
	// $31-$32 STREND
	bus->RAM[0x31] = en_addr & 0xFF;//0x00;
	bus->RAM[0x32] = (en_addr >> 0x8) & 0xFF;//0xA0;
	// $33-$34 FRETOP
	bus->RAM[0x33] = en_addr & 0xFF;
	bus->RAM[0x34] = (en_addr >> 0x8) & 0xFF;

	printf("LOADING DONE\n");*/
}

void	exec_clr() {
	_bus *bus = (_bus*)t_data->bus;

	bus->t_data->reset = TRUE;
}

FILE*	exec_ldd(FILE *file, char *file_path) {
	/*
		quietly replace .d64 FILE pointer
		and change the path string 
		to the new extracted .prg
	*/
	printf("\t- extracting first PRG file from D64 disk image..\n");
	file = read_d64file(file, file_path);
	if (file)
		printf("\t- found prg, storing at \"%s\"..\n", file_path);
	return file;
}

FILE	*exec_ldt(FILE *file, char *file_path) {
	(void)file_path;
	fclose(file);
	printf(":%serror:%s LDT is not implemented\n", RED, RST);
	return FALSE;
}
