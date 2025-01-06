#include "metallc64.h"

thread_data	*t_data;

void	sig_handle(int s) {
	pthread_cancel(t_data->worker);
	pthread_cancel(t_data->worker_2);
	pthread_join(t_data->worker, NULL);
	pthread_join(t_data->worker_2, NULL);

	/// / //		CLEAN
	pthread_mutex_destroy(&t_data->halt_mutex);
	pthread_mutex_destroy(&t_data->prg_mutex);
	pthread_mutex_destroy(&t_data->cmd_mutex);

	_bus	*bus = (_bus*)t_data->bus;
	_6502	*mos6502 = (_6502*)bus->cpu;
	_VIC_II	*vic = (_VIC_II*)bus->vic;
	_CIA	*cia1 = (_CIA*)bus->cia1;
	_CIA	*cia2 = (_CIA*)bus->cia2;
	_keymap	*keys = (_keymap*)cia1->keys;

	memset(bus->RAM, 0, sizeof(bus->RAM));
	mlx_delete_image(vic->mlx_ptr, vic->mlx_img);
	mlx_terminate(vic->mlx_ptr);

	if (t_data->line) free(t_data->line);
	if (bus->prg) free(bus->prg);
	if (bus->cmd) free(bus->cmd);
	free(mos6502);
	free(keys);
	free(vic);
	free(cia1);
	free(cia2);
	free(bus);
	free(t_data);
	exit(s);
}

int	main(int c, char **v) {
	if (c != 1) {
		printf("USAGE: %s\n", v[0]);
		return 1;
	}
	srand(time(0));

	/// / //		BUS
	_bus	*bus = malloc(sizeof(_bus));
	if (!bus) 
		return 1;
	bus->reset = bus_init;
	bus->reset(bus);
	if (!bus->load_roms(bus)) {
		free(bus);
		return 1;
	}

	//// / /		CPU
	_6502	*mos6502 = malloc(sizeof(_6502));
	if (!mos6502) { 
		free(bus);
		return 1;
	}
	mos6502->reset = cpu_init;
	mos6502->reset(mos6502, bus);
	bus->cpu = mos6502;

	// // //		VIC-II
	_VIC_II	*vic = malloc(sizeof(_VIC_II));
	if (!vic) {
		free(bus);
		free(mos6502);
		return 1;
	}
	vic->init = vic_init;
	vic->init(bus, vic);
	bus->vic = vic;

	/// // /		KEYMAP
	_keymap		*keys = malloc(sizeof(_keymap));
	if (!keys) {
		free(bus);
		free(vic);
		free(mos6502);
		return 1;
	}
	memset(keys, 0, sizeof(_keymap));
	memset(keys->matrix, 0xFF, sizeof(_keymap));
	
	/// / //		CIAs
	_CIA		*CIA1 = malloc(sizeof(_CIA));
	_CIA		*CIA2 = malloc(sizeof(_CIA));
	if (!CIA1 || !CIA2) {
		free(keys);
		free(vic);
		free(bus);
		free(mos6502);
		if (CIA1) free(CIA1);
		if (CIA2) free(CIA2);
		return 1;
	}
	CIA1->init = cia_init;
	CIA2->init = cia_init;
	CIA1->init(CIA1, 0xDC);
	CIA2->init(CIA2, 0xDD);
	CIA1->keys = keys;
	bus->cia1 = CIA1;
	bus->cia2 = CIA2;

	// /// /		THREAD INFO
	t_data = malloc(sizeof(thread_data));
	if (!t_data) {
		free(bus);
		free(vic);
		free(keys);
		free(mos6502);
		free(CIA1);
		free(CIA2);
		return 1;
	}
	memset(t_data, 0, sizeof(thread_data));
	pthread_mutex_init(&t_data->halt_mutex, NULL);
	pthread_mutex_init(&t_data->prg_mutex, NULL);
	pthread_mutex_init(&t_data->cmd_mutex, NULL);
	t_data->bus = bus;
	bus->t_data = t_data;

	// / ///		GRAPHIC WINDOW
	vic->wpdx = WPDX;
	vic->wpdy = WPDY;
	vic->win_height = WHEIGHT;
	vic->win_width = WWIDTH;
	vic->mlx_ptr = mlx_init(vic->win_width, vic->win_height, "MetallC64", true);
	if (!vic->mlx_ptr
	|| !(vic->mlx_img = mlx_new_image(vic->mlx_ptr, vic->win_width, vic->win_height))) {
		free(bus);
		free(vic);
		free(mos6502);
		free(keys);
		free(CIA1);
		free(CIA2);
		free(t_data);
		return 1;
	}
	draw_bg(vic, 0x0000FFFF);

	/// / //		CYCLE
	pthread_create(&t_data->worker, NULL, main_cycle, bus);

	/// / //		SHELL
	pthread_create(&t_data->worker_2, NULL, open_shell, bus);

	// / //		CLEAN
	signal(SIGINT, sig_handle);
	signal(SIGQUIT, sig_handle);
	signal(SIGTERM, sig_handle);
	
	//// / //		HOOKS
	mlx_image_to_window(vic->mlx_ptr, vic->mlx_img, 0, 0);
	mlx_set_window_limit(vic->mlx_ptr, WWIDTH, WHEIGHT, WWIDTH, WHEIGHT);
		// limited window dimensions for now
	setup_mlx_hooks(vic);
	mlx_loop(vic->mlx_ptr);
}
