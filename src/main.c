#include "metallc64.h"

thread_data	*t_data;

void	sig_handle(int s) {
	pthread_mutex_lock(&t_data->halt_mutex);
	t_data->halt = 1;
	pthread_mutex_unlock(&t_data->halt_mutex);
	pthread_join(t_data->worker, NULL);
	printf("parent thread: i killed'em\n");
	/// / //		CLEAN
	pthread_mutex_destroy(&t_data->halt_mutex);
	pthread_mutex_destroy(&t_data->data_mutex);

	_bus	*bus = (_bus*)t_data->bus;
	_6502	*mos6502 = (_6502*)bus->cpu;
	_VIC_II	*vic = (_VIC_II*)bus->vic;
	_CIA	*cia1 = (_CIA*)bus->cia1;
	_CIA	*cia2 = (_CIA*)bus->cia2;

	memset(bus->RAM, 0, sizeof(bus->RAM));
	mlx_delete_image(vic->mlx_ptr, vic->mlx_img);
	mlx_terminate(vic->mlx_ptr);
	free(mos6502);
	free(vic);
	free(cia1);
	free(cia2);
	free(bus);
	free(t_data);
	exit(s);
}

int	main() {
	srand(time(0));

	/// / //		BUS
	_bus	*bus = malloc(sizeof(_bus));
	if (!bus) 
		return 1;
	bus->reset = bus_init;
	bus->reset(bus);

	// / ///		BASIC
	if (!bus->load_basic(bus)) {
		printf("failed to load basic:%s\n", BASIC_PATH);
		free(bus);
		return 1;
	}

	/// / //		KERNAL
	if (!bus->load_kernal(bus)) {
		printf("failed to load kernal: %s\n", KERNAL_PATH);
		free(bus);
		return 1;
	}

	//// / //		CHARACTERS ROM
	if (!bus->load_chars(bus)) {
		printf("failed to load characters ROM: %s\n", CHAR_ROM_PATH);
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

	/// / //		CIAs
	_CIA		*CIA1 = malloc(sizeof(_CIA));
	_CIA		*CIA2 = malloc(sizeof(_CIA));
	if (!CIA1 || !CIA2) {
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
	bus->cia1 = CIA1;
	bus->cia2 = CIA2;

	// /// /		THREAD INFO
	t_data = malloc(sizeof(thread_data));
	if (!t_data) {
		free(bus);
		free(vic);
		free(mos6502);
		free(CIA1);
		free(CIA2);
		return 1;
	}
	memset(t_data, 0, sizeof(thread_data));
	pthread_mutex_init(&t_data->halt_mutex, NULL);
	pthread_mutex_init(&t_data->data_mutex, NULL);
	t_data->bus = bus;
	bus->t_data = t_data;

	// / ///		GRAPHIC WINDOW
	vic->win_height = WHEIGHT;
	vic->win_width = WWIDTH;
	vic->mlx_ptr = mlx_init(vic->win_width, vic->win_height, "MetallC64", true);
	if (!vic->mlx_ptr
	|| !(vic->mlx_img = mlx_new_image(vic->mlx_ptr, vic->win_width, vic->win_height))) {
		free(bus);
		free(vic);
		free(mos6502);
		free(CIA1);
		free(CIA2);
		free(t_data);
		return 1;
	}
	draw_bg(vic, 0x0000FFFF);

	/// / //		CYCLE
	pthread_create(&t_data->worker, NULL, main_cycle, bus);

	// / //		CLEAN
	signal(SIGINT, sig_handle);
	signal(SIGQUIT, sig_handle);
	signal(SIGTERM, sig_handle);
	
	//// / //		HOOKS
	mlx_image_to_window(vic->mlx_ptr, vic->mlx_img, 0, 0);
	mlx_set_window_limit(vic->mlx_ptr, MWIDTH, MHEIGHT, INT_MAX, INT_MAX);
	setup_mlx_hooks(vic);
	mlx_loop(vic->mlx_ptr);
}
