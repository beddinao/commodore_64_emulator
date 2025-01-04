#include "metallc64.h"

void	change_col(_bus* bus, char *cmd, unsigned col_i) {
	if (!strncmp(cmd, "BRD", 3))
		bus->RAM[BRD_COLOR] = col_i;
	else if (!strncmp(cmd, "BGR", 3))
		bus->RAM[BACKG_COLOR0] = col_i;
	else	bus->RAM[BRD_COLOR] = col_i;
}

bool	prg_load_sequence(_bus *bus, char *buffer, unsigned prg_size, char *filename) {
	_prg *prg = malloc(sizeof(_prg));
	if (!prg) return FALSE;
	prg->ld_addr = BASIC_PRG_START;
	prg->en_addr = BASIC_PRG_START + prg_size + 1;
	memcpy(prg->path, filename, strlen(filename));
	prg->size = prg_size;
	/* clear region */
	memset(bus->RAM + BASIC_PRG_START, 0, prg->size);
	/* load to RAM */
	memcpy(bus->RAM + BASIC_PRG_START, buffer, prg->size);
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
	bus->prg = prg;
	return TRUE;
}

void	reset_prg(_bus *bus) {
	_prg *prg = (_prg*)bus->prg;
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

void	exec_ldp(_bus *bus, char *cmd) {
	unsigned size = strlen(cmd);
	char *cmd_end = strstr(cmd, " ");
	char *path_st = cmd_end, *path_en;
	char buffer[BASIC_PRG_SIZE];
	unsigned chars_read, ld_addr;
	char file_path[0x400];
	FILE *file;

	for (unsigned i = cmd_end - cmd; i < size && *path_st == ' '; i++, path_st++);
	if (size > 0x400 || !(*cmd_end)) {
		printf("invalid file path \"%s\"\n", cmd_end);
		return;
	}

	path_en = &cmd[size];
	while (!isalnum(*path_en) && path_en > path_st)
		path_en--;
	memset(file_path, 0, sizeof(file_path));
	memcpy(file_path, path_st, (path_en - path_st) + 1);
	file = fopen(file_path, "rb");
	if (!file) {
		printf("failed to open file \"%s\"\n", file_path);
		return;
	}
	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, 2, file);
	if (!chars_read || chars_read != 2) {
		printf("invalid .prg BASIC file format\n");
		fclose(file);
		return;
	}
	ld_addr = buffer[1] << 0x8 | buffer[0];
	if (!ld_addr || ld_addr != BASIC_PRG_START) {
		printf("invalid BASIC program load address $%04X(.prg first two bytes):\n\
				must be inside BASIC prg area $%04X - $%04X\n\
				typically at $801\n",
				ld_addr, BASIC_PRG_START, BASIC_PRG_END);
		fclose(file);
		return;
	}
	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, BASIC_PRG_SIZE, file);
	if (!chars_read || ld_addr + chars_read > BASIC_PRG_END || ld_addr + chars_read < BASIC_PRG_START) {
		printf("invalid BASIC program size $%04X, += load-address($%04X) > $%04X:\n\
				must fit inside the specified BASIC prg area $%04X - $%04X\n",
				chars_read, ld_addr, BASIC_PRG_END, BASIC_PRG_START, BASIC_PRG_END);
		fclose(file);
		return;
	}
	fclose(file);
	printf("loading [%s] with size $%04X at $%04X, $801 -> $%04X ...\n",
			file_path, chars_read, ld_addr, ld_addr + chars_read);
	if (prg_load_sequence(bus, buffer, chars_read, file_path))
		printf("BASIC program loaded to RAM successfully\n");
	else	printf("BASIC program failed to load to RAM\n");
}

void	exec_ldd(_bus *bus, char *cmd) {
	(void)bus;
	(void)cmd;
	printf("executing LDD\n");
}

void	print_help(char *line) {
	printf("invalid syntax\"%s\"\n", line);
	printf("aviable commands: LDP LDD CLR EXT BGR TXT BRD\n");
}

void	print_col_help(char *line) {
	printf("invalid syntax\"%s\"\n", line);
	printf("colors are indexes between 0 and 15\n");
}

/*
   general parse res:
0: valid (add to history)
1: print help
2: print color help
3: do nothing
*/

uint8_t	parse_line(char *line, _bus *bus) {
	unsigned size = strlen(line);
	unsigned cmd_size;
	char *cmd_end;
	int col;

	if (size < 3) return 3;
	cmd_end = strstr(line, " ");
	if (!cmd_end) {
		if (!strcmp(line, "CLR"))
			reset_prg(bus);
		else if (!strcmp(line, "EXT")) {
			printf("exiting..\n");
			pthread_mutex_lock(&bus->t_data->halt_mutex);
			bus->t_data->halt = 1;
			pthread_mutex_unlock(&bus->t_data->halt_mutex);
		}
		else 	return 1;
		return 0;
	}
	else if ((cmd_size = cmd_end - line) != 3)
		return 1;
	if (!strncmp(line, "LDP", cmd_size))
		exec_ldp(bus, line);
	else if (!strncmp(line, "LDD", cmd_size))
		exec_ldd(bus, line);
	else if (!strncmp(line, "BRD", cmd_size)
			|| !strncmp(line, "BGR", cmd_size)
			|| !strncmp(line, "TXT", cmd_size)) {
		col = atoi(line + cmd_size);
		if (col < 0 || col > 15) 
			return 2;
		change_col(bus, line, col);
	}
	else return 3;
	return 0;
}

void	*open_shell(void *p) {
	_bus	*bus = (_bus*)p;
	uint8_t	parse_res;

	while (1) {
		bus->t_data->line = readline("  $> ");
		if (!(parse_res = parse_line(bus->t_data->line, bus)))
			add_history(bus->t_data->line);
		else	switch (parse_res) {
			case 1: print_help(bus->t_data->line); break;
			case 2: print_col_help(bus->t_data->line); break;
			default: break;
		}
		free(bus->t_data->line);
		bus->t_data->line = NULL;
	}
	return NULL;
}

