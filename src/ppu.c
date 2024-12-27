#include "metallc64.h"

/*
	Dummy PPU routine for debugging
*/

void	loop_hook(void *p) {
	_ppu	*ppu = (_ppu*)p;
	_bus	*bus = ppu->bus;
	////
	(void)bus;
	////
}	
