#include "metallc64.h"

void	exec_ldp(_bus *bus, char *cmd) {
	if (bus->prg) {
		printf("a program is already loaded\n");
		return;
	}

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

void	exec_ldd(_bus *bus, char *cmd) {
	(void)bus;
	(void)cmd;
	printf("executing LDD\n");
}

void	exec_ldt(_bus *bus, char *cmd) {
	(void)bus;
	(void)cmd;
	printf("executing LDT\n");
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
	if (!strncmp(line, "LDP", cmd_size))
		exec_ldp(bus, line);
	else if (!strncmp(line, "LDD", cmd_size))
		exec_ldd(bus, line);
	else if (!strncmp(line, "LDT", cmd_size))
		exec_ldt(bus, line);
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

