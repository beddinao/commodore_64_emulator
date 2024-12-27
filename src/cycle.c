#include "metallc64.h"

void	ppu_step(void *p) {
	_bus	*bus = (_bus*)p;
	_VIC_II	*ppu = (_VIC_II*)bus->ppu;
	_6502	*cpu = (_6502*)bus->cpu;
	///
	(void)cpu;
	(void)ppu;
	///
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
   Fuck the NES 

   8000  78 D8 A9  	A:00 X:00 Y:00 P:24 SP:FD	SEI_IMP
   8001  D8 A9 10  	A:00 X:00 Y:00 P:24 SP:FD	CLD_IMP
   8002  A9 10 8D  	A:00 X:00 Y:00 P:24 SP:FD	LDA_IMM
   8004  8D 00 20  	A:10 X:00 Y:00 P:24 SP:FD	STA_ABScpu writing: 0x2000 == 0x10
   8007  A2 FF 9A  	A:10 X:00 Y:00 P:24 SP:FD	LDX_IMM
   8009  9A AD 02  	A:10 X:FF Y:00 P:A4 SP:FD	TXS_IMP
   800A  AD 02 20  	A:10 X:FF Y:00 P:A4 SP:FF	cpu reading: 0x2002 == 0x80	LDA_ABS
   800D  10 FB AD  	A:80 X:FF Y:00 P:A4 SP:FF	BPL_REL
   800F  AD 02 20  	A:80 X:FF Y:00 P:A4 SP:FF	cpu reading: 0x2002 == 0x80	LDA_ABS
   8012  10 FB A0  	A:80 X:FF Y:00 P:A4 SP:FF	BPL_REL
   8014  A0 FE A2  	A:80 X:FF Y:00 P:A4 SP:FF	LDY_IMM
   8016  A2 05 BD  	A:80 X:FF Y:FE P:A4 SP:FF	LDX_IMM
   8018  BD D7 07  	A:80 X:05 Y:FE P:24 SP:FF	LDA_ABSX
   801B  C9 0A B0  	A:00 X:05 Y:FE P:26 SP:FF	CMP_IMM
   801D  B0 0C CA  	A:00 X:05 Y:FE P:A4 SP:FF	BCS_REL
   801F  CA 10 F6  	A:00 X:05 Y:FE P:A4 SP:FF	DEX_IMP
   8020  10 F6 AD  	A:00 X:04 Y:FE P:24 SP:FF	BPL_REL
   8018  BD D7 07  	A:00 X:04 Y:FE P:24 SP:FF	LDA_ABSX
   801B  C9 0A B0  	A:00 X:04 Y:FE P:26 SP:FF	CMP_IMM
   801D  B0 0C CA  	A:00 X:04 Y:FE P:A4 SP:FF	BCS_REL
   801F  CA 10 F6  	A:00 X:04 Y:FE P:A4 SP:FF	DEX_IMP
   8020  10 F6 AD  	A:00 X:03 Y:FE P:24 SP:FF	BPL_REL
   8018  BD D7 07  	A:00 X:03 Y:FE P:24 SP:FF	LDA_ABSX
   801B  C9 0A B0  	A:00 X:03 Y:FE P:26 SP:FF	CMP_IMM

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

