#include "metallc64.h"

thread_data	*t_data;

void	exit_handle(int s) {
	pthread_cancel(t_data->worker);
	pthread_join(t_data->worker, NULL);

	printf("\nexiting..\n");
	/// / //		CLEAN
	pthread_mutex_destroy(&t_data->halt_mutex);
	pthread_mutex_destroy(&t_data->prg_mutex);
	pthread_mutex_destroy(&t_data->cmd_mutex);

	_bus	*bus = (_bus*)t_data->bus;
	_VIC_II	*vic = (_VIC_II*)bus->vic;

	memset(bus->RAM, 0, sizeof(bus->RAM));
	memset(bus->BASIC, 0, sizeof(bus->BASIC));
	memset(bus->KERNAL, 0, sizeof(bus->KERNAL));
	memset(bus->CHARACTERS, 0, sizeof(bus->CHARACTERS));

	SDL_DestroyRenderer(vic->renderer);
	SDL_DestroyWindow(vic->win);
	SDL_Quit();

	if (t_data->line) free(t_data->line);
	if (bus->prg) free(bus->prg);
	if (bus->cmd) free(bus->cmd);
	bus->clean(bus);
	free(t_data);
	free(bus);
	exit(s);
}

thread_data *t_data_init(_bus *bus) {
	t_data = malloc(sizeof(thread_data));
	if (!t_data) {
		bus->clean(bus);
		free(bus);
		return FALSE;
	}
	memset(t_data, 0, sizeof(thread_data));
	pthread_mutex_init(&t_data->halt_mutex, NULL);
	pthread_mutex_init(&t_data->prg_mutex, NULL);
	pthread_mutex_init(&t_data->cmd_mutex, NULL);
	t_data->bus = bus;
	return t_data;
}

int	main() {
	srand(time(0));

	/// / //		BUS
	_bus	*bus = bus_init();
	if (!bus) 
		return 1;
	
	//// / /		CPU
	bus->cpu = cpu_init(bus);
	if (!bus->cpu)
		return 1;

	// // //		VIC-II
	bus->vic = vic_init(bus);
	if (!bus->vic)
		return 1;

	/// / //		CIAs/TODs/KEYMAP
	bus->cia1 = cia_init(bus, 0xDC);
	bus->cia2 = cia_init(bus, 0xDD);
	if (!bus->cia1 || !bus->cia2)
		return 1;

	// /// /		THREAD INFO
	bus->t_data = t_data_init(bus);
	if (!bus->t_data)
		return 1;

	// / ///		GRAPHIC WINDOW
	((_VIC_II*)bus->vic)->win = init_window(bus, bus->vic);
	if (!((_VIC_II*)bus->vic)->win) {
		free(t_data);
		return 1;
	}

	// // /		CLEAN
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/// / //		SHELL

	//// / //		CYCLE
	emscripten_set_main_loop_args(main_cycle, bus, 0, 1);
}
