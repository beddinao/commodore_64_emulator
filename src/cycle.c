#include "metallc64.h"

void	ppu_step(void *p) {
	_bus	*bus = (_bus*)p;
	_VIC_II	*ppu = (_VIC_II*)bus->ppu;
	_6502	*cpu = (_6502*)bus->cpu;
	///
	(void)ppu;
	(void)cpu;
}

uint8_t	NMI_interrupt(_bus *bus, _6502 *mos6502) {
	if (mos6502->nmi_pending) {
		uint8_t	status;
		uint16_t	nmi_vector_low, nmi_vector_high;

		mos6502->push(mos6502, (mos6502->PC >> 8) & 0xFF);
		mos6502->push(mos6502, mos6502->PC & 0xFF);

		status = mos6502->SR;
		status &= ~0x10; 
		status |= 0x20;
		mos6502->push(mos6502, status);

		mos6502->set_flag(mos6502, 'I', 1);

		nmi_vector_low = bus->cpu_read(bus, NMI);
		nmi_vector_high = bus->cpu_read(bus, NMI + 1);
		mos6502->PC = (nmi_vector_high << 8) | nmi_vector_low;

		mos6502->nmi_pending = 0;
		return 7;
	}
	return 0;
}

// /// /	CYCLE	1ppu-step = 1cpu-step

void	*instruction_cycle(void *p) {
	_bus	*bus = (_bus*)p;
	_6502	*mos6502 = (_6502*)bus->cpu;

	while (1) {
		if (mos6502->cycles) {
			mos6502->cycles--;
			continue;
		}
		//	Sync
		pthread_mutex_lock(&bus->t_data->halt_mutex);
		if (bus->t_data->halt)
			break;
		pthread_mutex_unlock(&bus->t_data->halt_mutex);

		//	INTERRUPTS
		mos6502->cycles = NMI_interrupt(bus, mos6502);

		//	CPU
		mos6502->opcode = bus->cpu_read(bus, mos6502->PC);
		mos6502->cycles = mos6502->opcodes[mos6502->opcode](mos6502);

		//	PPU
		ppu_step(bus);
	}
	pthread_mutex_unlock(&bus->t_data->halt_mutex);
	printf("second thread: i died\n");
	return NULL;
}



/*
   the NES donkey kong ROM unfortunate result

   D8 CLD (implied)	1byte	set_flag('D', 0)
   78 SEI (implied)	1byte	set_flag('I', 1)
   AD LDA (absolute)	3bytes	load_accumulator(PC+1, PC+2)
   10 BPL (relative)	2bytes	PC += val in operand addr
   AD LDA (absolute)	3bytes	load_accumulator(PC+1, PC+2)
   10 BPL (relative)	2bytes	PC += val in operand addr
   A2 LDX (immediate)	2bytes	load_X(PC+1)
   8E STX (absolute)	3bytes	store_X(PC+1, PC+2)
   8E STX (absolute)	3bytes	store_X(PC+1, PC+2)
   CA DEX (implied)	1byte	X -= 1
   9A TXS (implied)	1byte	SR = X
   A9 LDA (immediate)	2bytes	load_accumulator(PC+1)
   20 JSR (absolute)	3bytes	Jump to New Location Saving Return Address
   17 illegal
   03 illegal

*/

