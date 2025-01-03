#include "metallc64.h"

/*
	Dummy PPU routine for debugging
*/


void	loop_hook(void *p) {
	_VIC_II	*ppu = (_VIC_II*)p;
	_bus	*bus = ppu->bus;
	////
	pthread_mutex_lock(&bus->t_data->halt_mutex);
	if (bus->t_data->halt) {
		pthread_mutex_unlock(&bus->t_data->halt_mutex);
		sig_handle(0);
	}
	pthread_mutex_unlock(&bus->t_data->halt_mutex);
	///
}	
