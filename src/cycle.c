#include "metallc64.h"

uint32_t	get_background_pixel(_bus *bus, unsigned x, unsigned y) {
	unsigned index = bus->ppu_read(bus, NMETAB_0 + y * T_HEIGHT + x);
	uint32_t color = bus->ppu_read(bus, PTRN_TAB + index);
	return color;
}

uint32_t	get_sprite_pixel(_ppu *ppu, unsigned x, unsigned y) {
	(void)x;
	(void)y;
	(void)ppu;
	return 0x3F6EFC;
}

void	ppu_step(void *p) {
	_bus	*bus = (_bus*)p;
	_ppu	*ppu = (_ppu*)bus->ppu;
	_6502	*cpu = (_6502*)bus->cpu;
	///

	if (ppu->scanline == 241 && ppu->cycle == 1) {
		ppu->status |= 0x80;
		if (ppu->ctrl & 0x80) {
			cpu->nmi_pending = 1;
		}
	}
	else if (ppu->scanline == 261 && ppu->cycle == 1) {
		ppu->status &= ~0x80;
		ppu->status &= ~0x40; // clear sprite 0 hit
		ppu->status &= ~0x20; // clear sprite overflow
	}

	if (ppu->mask & 0x8) {
		if (ppu->scanline < P_HEIGHT && ppu->cycle < P_WIDTH) {
			unsigned x = ppu->cycle;
			unsigned y = ppu->scanline;
			uint32_t color = get_background_pixel(bus, x, y);

			if (ppu->mask & 0x10) {
				uint32_t sprite_color = get_sprite_pixel(ppu, x, y);
				if (sprite_color)
					color = sprite_color;
			}
			
			ppu->screen[y * P_WIDTH + x] = color;
		}
	}

	if (ppu->scanline == 261 && ppu->cycle == 340) {
		render_screen(ppu);
	}

	ppu->cycle++;
	if (ppu->cycle > 340) {
		ppu->cycle = 0;
		ppu->scanline++;
		if (ppu->scanline > 261) {
			ppu->scanline = 0;
		}
	}

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

		nmi_vector_low = bus->cpu_read(bus, NMI + RAM_SIZE);
		nmi_vector_high = bus->cpu_read(bus, NMI + RAM_SIZE + 1);
		mos6502->PC = (nmi_vector_high << 8) | nmi_vector_low;

		mos6502->nmi_pending = 0;
		return 7;
	}
	return 0;
}

// /// /	CYCLE	3ppu-step = 1cpu-step

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
		ppu_step(bus);
		ppu_step(bus);
	}
	pthread_mutex_unlock(&bus->t_data->halt_mutex);
	printf("second thread: i died\n");
	return NULL;
}



/*
   the donkey kong ROM unfortunate result

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

/*
   printf("%04X  %02X %02X %02X  ",
   mos6502->PC, mos6502->opcode,
   bus->cpu_read(bus, mos6502->PC+1), bus->cpu_read(bus, mos6502->PC+2));
   printf("\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\t",
   mos6502->A, mos6502->X, mos6502->Y, mos6502->SR, mos6502->SP);
  */

	/*for (unsigned row = 0; row < T_HEIGHT; row++) {
		if (!row) {
			printf("   ");
			for (unsigned col = 0; col < T_WIDTH; col++)
				printf("%s%02X%s ", UND, col, RST);
			printf("\n");
		}
		///
		printf("%s%02X%s ", UND, row, RST);
		for (unsigned col = 0; col < T_WIDTH; col++) {
			unsigned index = bus->ppu_read(bus, NMETAB_0 + row*T_HEIGHT + col);
			printf("%s%02X%s ", WHT, index, RST);
			//printf("%s%02X%s ", WHT, bus->ppu_read(bus, PTRN_TAB + index), RST);
		}
		printf("\n");
	}
	printf("\n");*/

