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

int	main(int c, char **v) {
	if (c != 2) {
		printf("Usage: %s [ROM]\n", v[0]);
		return 1;
	}
	srand(time(0));

	/// / //		BUS
	_bus	*bus = malloc(sizeof(_bus));
	if (!bus) 
		return 1;
	memset(bus, 0, sizeof(_bus));
	bus->reset = bus_init;
	bus->reset(bus);

	// / ///		ROM
	if (!bus->load_ROM(bus, v[1])) {
		printf("failed to load program to memory\n");
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

	// // //		PPU
	_VIC_II	*ppu = malloc(sizeof(_VIC_II));
	if (!ppu) {
		free(bus);
		free(mos6502);
		return 1;
	}
	memset(ppu, 0, sizeof(_VIC_II));
	ppu->bus = bus;
	bus->ppu = ppu;
	
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
	ppu->win_height = WIN_HEIGHT;
	ppu->win_width = WIN_WIDTH;
	ppu->mlx_ptr = mlx_init(ppu->win_width, ppu->win_height, "MetalNES", true);
	if (!ppu->mlx_ptr
	|| !(ppu->mlx_img = mlx_new_image(ppu->mlx_ptr, ppu->win_width, ppu->win_height))) {
		free(bus);
		free(ppu);
		free(mos6502);
		free(t_data);
		return 1;
	}
	draw_bg(ppu, 0x000000);

	/// / //		CYCLE
	pthread_create(&t_data->worker, NULL, mos6502->instruction_cycle, bus);

	// / //		CLEAN
	signal(SIGINT, sig_handle);
	signal(SIGQUIT, sig_handle);
	signal(SIGTERM, sig_handle);
	
	//// / //		HOOKS
	mlx_image_to_window(ppu->mlx_ptr, ppu->mlx_img, 0, 0);
	setup_mlx_hooks(ppu);
	mlx_set_window_limit(ppu->mlx_ptr, MIN_WIDTH, MIN_HEIGHT, INT_MAX, INT_MAX);
	mlx_loop(ppu->mlx_ptr);
}
