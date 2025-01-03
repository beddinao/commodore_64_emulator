#include "metallc64.h"

void	change_col(_bus* bus, char *cmd, unsigned col_i) {
	if (!strncmp(cmd, "BRD", 3))
		bus->RAM[BRD_COLOR] = col_i;
	else if (!strncmp(cmd, "BGR", 3))
		bus->RAM[BACKG_COLOR0] = col_i;
	else	bus->RAM[BRD_COLOR] = col_i;
}

void	exec_ldp(_bus *bus, char *cmd) {
	(void)bus;
	(void)cmd;
	printf("executing ldp\n");
}

void	exec_ldd(_bus *bus, char *cmd) {
	(void)bus;
	(void)cmd;
	printf("executing ldd\n");
}

/*
	parse_res:
		0: valid (add to history)
		1: print help
		2: print color help
		3: do nothing

*/

void	print_help(char *line) {
	printf("invalid syntax\"%s\"\n", line);
	printf("aviable commands: LDP LDD CLR EXT BGR TXT BRD\n");
}

void	print_col_help(char *line) {
	printf("invalid syntax\"%s\"\n", line);
	printf("colors are indexes between 0 and 15\n");
}

uint8_t	parse_line(char *line, _bus *bus) {
	unsigned size = strlen(line);
	unsigned cmd_size;
	char *cmd_end;
	int col;

	if (size < 3) return 3;
	cmd_end = strstr(line, " ");
	if (!cmd_end) {
		if (!strcmp(line, "CLR"))
			printf("clearing prg data\n");
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

