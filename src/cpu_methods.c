#include "metallc64.h"

/// / //	STACK
// >>>>low(00FF)>>>high(FF00)>>>> 

void	stack_push(_6502 *mos6502, uint8_t val) {
	mos6502->bus->cpu_write(mos6502->bus, STACK_START + mos6502->SP, val);
	mos6502->SP--;
}

uint8_t	stack_pull(_6502 *mos6502) {
	mos6502->SP++;
	return mos6502->bus->cpu_read(mos6502->bus, STACK_START + mos6502->SP);
}

// /////	STATUS FLAGS

void	set_flag(_6502 *mos6502, uint8_t pos, uint8_t bit) {
	uint8_t	sr = mos6502->SR;
	switch (pos) {
		///	N V A B  D I Z C
		///	0 1 1 0  1 1 1 1 6F
		///	0 1 1 1  1 1 1 1 7F
		///
		///	0 0 1 0  0 1 1 1 27
		///	0 1 1 0  0 1 1 1 67
		///
		case 'N':	bit ? (sr|=0x80) : (sr&=~0x80); break;
		case 'V': bit ? (sr|=0x40) : (sr&=~0x40); break;
		case 'A': bit ? (sr|=0x20) : (sr&=~0x20); break;
		case 'B':	bit ? (sr|=0x10) : (sr&=~0x10); break;
		case 'D':	bit ? (sr|=0x8) : (sr&=~0x8); break;
		case 'I':	bit ? (sr|=0x4) : (sr&=~0x4); break;
		case 'Z':	bit ? (sr|=0x2) : (sr&=~0x2); break;
		case 'C':	bit ? (sr|=0x1) : (sr&=~0x1); break;
		default: break;
	}
	mos6502->SR = sr;
}

uint8_t	get_flag(_6502* mos6502, uint8_t pos) {
	uint8_t	sr = mos6502->SR;
	switch (pos) {
		case 'N': return (sr >> 0x7) & 0x1;
		case 'V': return (sr >> 0x6) & 0x1;
		case 'A': return (sr >> 0x5) & 0x1;
		case 'B': return (sr >> 0x4) & 0x1;
		case 'D': return (sr >> 0x3) & 0x1;
		case 'I': return (sr >> 0x2) & 0x1;
		case 'Z': return (sr >> 0x1) & 0x1;
		case 'C': return sr & 0x1;
		default: return 0;
	}
}

/// // /	RESET

void	cpu_init(_6502* mos6502) {
	_bus *bus = mos6502->bus;
	mos6502->pull = stack_pull;
	mos6502->push = stack_push;
	mos6502->set_flag = set_flag;
	mos6502->get_flag = get_flag;
	load_instructions(mos6502);
	mos6502->instruction_cycle = instruction_cycle;
	mos6502->PC = bus->cpu_read(bus, bus->rstv + RAM_SIZE + 1) << 8 |
		bus->cpu_read(bus, bus->rstv + RAM_SIZE);
	/*printf("rstv: %04X-%04X, init addr: %04X, PC == %04X\n\n",
		bus->rstv, bus->rstv + 1,
		bus->cartridge[bus->rstv+1] << 8|bus->cartridge[bus->rstv],
		mos6502->PC);*/
	mos6502->opcode = 0x0;
	mos6502->cycles = 0x0;
	mos6502->SP = 0xFD;
	mos6502->SR = 0x0;
	mos6502->A = 0x0;
	mos6502->X = 0x0;
	mos6502->Y = 0x0;
	mos6502->set_flag(mos6502, 'I', 1);
	mos6502->set_flag(mos6502, 'A', 1);
}
