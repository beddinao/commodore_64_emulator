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
	mlx_delete_image(((_VIC_II*)t_data->ppu)->mlx_ptr, ((_VIC_II*)t_data->ppu)->mlx_img);
	mlx_terminate(((_VIC_II*)t_data->ppu)->mlx_ptr);
	free(t_data->ppu);
	free(t_data->cpu);
	free(t_data);
	exit(s);
}

int	main() {
	srand(time(0));

	/// / //		BUS
	_bus	*bus = malloc(sizeof(_bus));
	if (!bus) 
		return 1;
	memset(bus, 0, sizeof(_bus));
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
	memset(mos6502, 0, sizeof(_6502));
	mos6502->bus = bus;
	mos6502->reset = cpu_init;
	mos6502->reset(mos6502);
	bus->cpu = mos6502;

	// // //		VIC-II
	_VIC_II	*ppu = malloc(sizeof(_VIC_II));
	if (!ppu) {
		free(bus);
		free(mos6502);
		return 1;
	}
	memset(ppu, 0, sizeof(_VIC_II));
	ppu->get_raster = get_raster;
	ppu->C64_to_rgb = C64_to_rgb;
	ppu->bus = bus;
	bus->ppu = ppu;
	ppu->char_ram = LOW_CHAR_ROM_START;
	ppu->screen_ram = DEFAULT_SCREEN;
	ppu->bitmap_ram = 0x0000;
	ppu->bank = VIC_BANK_0;

	/// / //		CIAs
	_CIA		*CIA1 = malloc(sizeof(_CIA));
	_CIA		*CIA2 = malloc(sizeof(_CIA));
	if (!CIA1 || !CIA2) {
		free(ppu);
		free(bus);
		free(mos6502);
		if (CIA1) free(CIA1);
		if (CIA2) free(CIA2);
		return 1;
	}
	memset(CIA1, 0, sizeof(_CIA));
	memset(CIA2, 0, sizeof(_CIA));
	bus->cia1 = CIA1;
	bus->cia2 = CIA2;

	// /// /		THREAD INFO
	t_data = malloc(sizeof(thread_data));
	if (!t_data) {
		free(bus);
		free(ppu);
		free(mos6502);
		return 1;
	}
	memset(t_data, 0, sizeof(thread_data));
	pthread_mutex_init(&t_data->halt_mutex, NULL);
	pthread_mutex_init(&t_data->data_mutex, NULL);
	t_data->cpu = mos6502;
	t_data->ppu = ppu;
	bus->t_data = t_data;

	// / ///		GRAPHIC WINDOW
	ppu->win_height = WHEIGHT;
	ppu->win_width = WWIDTH;
	ppu->mlx_ptr = mlx_init(ppu->win_width, ppu->win_height, "MetallC64", true);
	if (!ppu->mlx_ptr
	|| !(ppu->mlx_img = mlx_new_image(ppu->mlx_ptr, ppu->win_width, ppu->win_height))) {
		free(bus);
		free(ppu);
		free(mos6502);
		free(t_data);
		return 1;
	}
	draw_bg(ppu, 0x0000FFFF);

	/// / //		CYCLE
	pthread_create(&t_data->worker, NULL, mos6502->instruction_cycle, bus);

	// / //		CLEAN
	signal(SIGINT, sig_handle);
	signal(SIGQUIT, sig_handle);
	signal(SIGTERM, sig_handle);
	
	//// / //		HOOKS
	mlx_image_to_window(ppu->mlx_ptr, ppu->mlx_img, 0, 0);
	mlx_set_window_limit(ppu->mlx_ptr, MWIDTH, MHEIGHT, INT_MAX, INT_MAX);
	setup_mlx_hooks(ppu);
	mlx_loop(ppu->mlx_ptr);
}
