#include "metallc64.h"

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
	sleep(1);

	return TRUE;
}

FILE	*get_binary_file(char *cmd, char *path) {
	unsigned size = strlen(cmd);
	char *cmd_end = strstr(cmd, " ");
	char *path_st = cmd_end, *path_en;
	FILE *file;

	for (unsigned i = cmd_end - cmd; i < size && *path_st == ' '; i++, path_st++);
	if (size > PATH_MAX_SIZE || !(*cmd_end)) {
		printf("invalid file path \"%s\"\n", cmd_end);
		return NULL;
	}

	path_en = &cmd[size];
	while (!isalnum(*path_en) && path_en > path_st)
		path_en--;
	memset(path, 0, PATH_MAX_SIZE);
	memcpy(path, path_st, (path_en - path_st) + 1);
	file = fopen(path, "rb");
	if (!file) {
		printf("failed to open file \"%s\"\n", path);
		return NULL;
	}
	return file;
}

void	exec_ldp(_bus *bus, FILE *file, char *file_path) {
	char buffer[BASIC_PRG_SIZE];
	unsigned chars_read, ld_addr;

	memset(buffer, 0, sizeof(buffer));
	chars_read = fread(buffer, 1, 2, file);
	if (!chars_read || chars_read != 2) {
		printf("invalid PRG file format\nhint: position indicator is at %li\n", ftell(file));
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

	_prg *prg = malloc(sizeof(_prg));
	if (!prg) {
		printf("BASIC program failed to load to RAM\n");
		return;
	}
	memset(prg, 0, sizeof(_prg));
	prg->size = chars_read;
	prg->ld_addr = BASIC_PRG_START;
	prg->en_addr = BASIC_PRG_START + chars_read + 1;
	memcpy(prg->path, file_path, strlen(file_path));
	memcpy(prg->buffer, buffer, chars_read);
	bus->prg = prg;

	pthread_mutex_lock(&bus->t_data->prg_mutex);
	bus->t_data->load = TRUE;
	pthread_mutex_unlock(&bus->t_data->prg_mutex);
	sleep(1);
}

FILE*	exec_ldd(FILE *file, char *file_path) {
	/*
		quietly replace .d64 FILE pointer
		and change the path string 
		to the new extracted .prg
	*/
	printf("\nextracting PRG file from D64 disk image\n");
	file = read_d64file(file, file_path);
	if (file)
		printf("PRG file is stored at \"%s\"\n", file_path);
	return file;
}

FILE	*exec_ldt(FILE *file, char *file_path) {
	(void)file_path;
	fclose(file);
	printf("LDT is not implemented\n");
	return FALSE;
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
	_cmd *cmd;
	int col;

	if (!size) return 3;
	cmd_end = strstr(line, " ");
	if (!cmd_end) {
		if (!strcmp(line, "CLR")) {
			if (!bus->prg) {
				printf("you must load a program first\n");
				return 0;
			}
			pthread_mutex_lock(&bus->t_data->prg_mutex);
			bus->t_data->reset = TRUE;
			pthread_mutex_unlock(&bus->t_data->prg_mutex);
			sleep(1);
		}
		else if (!strcmp(line, "EXT")) {
			printf("exiting..\n");
			pthread_mutex_lock(&bus->t_data->halt_mutex);
			bus->t_data->halt = TRUE;
			pthread_mutex_unlock(&bus->t_data->halt_mutex);
			pthread_exit(NULL);
		}
		else if (!strcmp(line, "SCR") || !strcmp(line, "SVR")
			|| !strcmp(line, "SC1") || !strcmp(line, "SC2")) {
			cmd = malloc(sizeof(_cmd));
			if (cmd) {
				memset(cmd, 0, sizeof(_cmd));
				memcpy(cmd->cmd, line, 3);
				bus->cmd = cmd;
				pthread_mutex_lock(&bus->t_data->cmd_mutex);
				bus->t_data->cmd = TRUE;
				pthread_mutex_unlock(&bus->t_data->cmd_mutex);
				sleep(1);
			}
			else printf("memory dump failed\n");
		}
		else 	return 1;
		return 0;
	}
	else if ((cmd_size = cmd_end - line) != 3)
		return 1;
	if (!strncmp(line, "LDP", cmd_size) || !strncmp(line, "LDD", cmd_size)
		|| !strncmp(line, "LDT", cmd_size)) {
		if (bus->prg) {
			printf("a program is already loaded\n");
			return 0;
		}
		char file_path[PATH_MAX_SIZE];
		FILE *file = get_binary_file(line, file_path);
		if (!file) return 0;
		if (!strncmp(line, "LDD", cmd_size) && !(file = exec_ldd(file, file_path))) return 0;
		if (!strncmp(line, "LDT", cmd_size) && !(file = exec_ldt(file, file_path))) return 0;
		exec_ldp(bus, file, file_path);
	}
	else if (!strncmp(line, "DMP", cmd_size)) {
		if (!exec_dmp(bus, line))
			printf("invalid DMP syntax\"%s\"\n\n \
				usage: DMP $addr1 $addr2,\n \
				where addr is a hex number between $0 and $FFFF\n \
				and 2 is bigger than 1\n\n", line);
	}
	else if (!strncmp(line, "BRD", cmd_size)
			|| !strncmp(line, "BGR", cmd_size)
			|| !strncmp(line, "TXT", cmd_size)) {
		col = atoi(line + cmd_size);
		if (col <= 0 || col > 16) 
			return 2;
		cmd = malloc(sizeof(_cmd));
		if (cmd) {
			memset(cmd, 0, sizeof(_cmd));
			memcpy(cmd->cmd, line, 3);
			cmd->col = col;
			bus->cmd = cmd;
			pthread_mutex_lock(&bus->t_data->cmd_mutex);
			bus->t_data->cmd = TRUE;
			pthread_mutex_unlock(&bus->t_data->cmd_mutex);
			sleep(1);
		}
		else printf("changing color failed\n");
	}
	else return 1;
	return 0;
}

void	*open_shell(void *p) {
	_bus	*bus = (_bus*)p;
	uint8_t	parse_res;

	while (1) {
		bus->t_data->line = readline(SHELL_PRMPT);
		if (!(parse_res = parse_line(bus->t_data->line, bus)))
			add_history(bus->t_data->line);
		else	
			switch (parse_res) {
				case 1: print_help(bus->t_data->line); break;
				case 2: print_col_help(bus->t_data->line); break;
				default: break;
		}
		free(bus->t_data->line);
		bus->t_data->line = NULL;
	}
	return NULL;
}

