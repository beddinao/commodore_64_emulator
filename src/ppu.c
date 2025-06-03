#include <c64_emu.h> 

/*
	Dummy PPU routine for debugging
*/


void	loop_hook(void *p) {
	_bus	*bus = (_bus*)p;
	////
	pthread_mutex_lock(&bus->t_data->halt_mutex);
	if (bus->t_data->halt) {
		pthread_mutex_unlock(&bus->t_data->halt_mutex);
		exit_handle(0);
	}
	pthread_mutex_unlock(&bus->t_data->halt_mutex);
	///
}	
