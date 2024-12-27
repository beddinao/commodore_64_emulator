#include "metallc64.h"

/*
	Dummy PPU routine for debugging
*/

void	loop_hook(void *p) {
	_VIC_II	*ppu = (_VIC_II*)p;
	_bus	*bus = ppu->bus;
	////
	(void)bus;
	////
}	
