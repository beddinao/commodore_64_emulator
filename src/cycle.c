#include "metallc64.h"

uint16_t	get_raster(_VIC_II *ppu) {
	uint8_t cntrl1 = ppu->bus->cpu_read(ppu->bus, CNTRL1);
	return ((cntrl1 >> 0x7) & 0x1) << 0x8 |
		ppu->bus->cpu_read(ppu->bus, RASTER);
}

void	increment_raster(_VIC_II *ppu, uint16_t raster) {
	raster++;
	if (raster > GHEIGHT)
		raster = 0;
	ppu->bus->cpu_write(ppu->bus, RASTER, raster & 0x00FF);

	uint8_t high_byte = (raster >> 0x8) & 0x1;
	uint8_t cntrl1 = ppu->bus->cpu_read(ppu->bus, CNTRL1);
	if (high_byte)
		cntrl1 |= 0x80;
	else	cntrl1 &= ~0x80;
	ppu->bus->cpu_write(ppu->bus, CNTRL1, cntrl1);
}

uint32_t	C64_to_rgb(uint8_t color) {
	uint32_t c64_colors[16] = {
		0x000000,
		0xFFFFFF,
		0x68372B,
		0x70A4B2,
		0x6F3D86,
		0x588D43,
		0x352879,
		0xB8C76F,
		0x6F4F25,
		0x433900,
		0x9A6759,
		0x444444,
		0x6C6C6C,
		0x9AD284,
		0x6C5EB5,
		0x959595,
	};
	return c64_colors[color & 0xF] << 0x8 | 0xFF;
}

uint8_t	ppu_step(void *p) {
	_VIC_II	*ppu = (_VIC_II*)p;
	_bus	*bus = (_bus*)ppu->bus;
	uint32_t brd_color = ppu->C64_to_rgb(bus->cpu_read(bus, BRD_COLOR)),
	         bg_color = ppu->C64_to_rgb(bus->cpu_read(bus, BACKG_COLOR0)), t_bg_color,
	         fg_color;

	if (ppu->raster < DYSTART || ppu->raster >= DYEND) {
		for (unsigned x = 0; x < GWIDTH; x++)
			mlx_put_pixel(ppu->mlx_img, x, ppu->raster, brd_color);
	}
	else {
		for (unsigned x = 0, row, col; x < GWIDTH; x++) {
			if (x < DXSTART || x >= DXEND)
				mlx_put_pixel(ppu->mlx_img, x, ppu->raster, brd_color);
			else {
				col = (x - DXSTART) / 0x8;
				row = (ppu->raster - DYSTART) / 0x8;
				if ((bus->cpu_read(bus, CNTRL1) >> 0x5) & 0x1) {
					uint8_t byte_offset = (row * 320) + (col * 8) + (ppu->raster % 8);
					uint8_t bitmap_data = bus->ppu_read(bus, ppu->bitmap_ram + byte_offset);
					uint8_t color_data = bus->ppu_read(bus, ppu->screen_ram + (row * 40 + col));
					uint8_t bit_pos = (x - DXSTART) % 0x8;
					if (bitmap_data & (0x80 >> bit_pos))
						fg_color = ppu->C64_to_rgb(color_data >> 0x4);
					else	fg_color = ppu->C64_to_rgb(color_data & 0xF);
					mlx_put_pixel(ppu->mlx_img, x, ppu->raster, fg_color);
				}
				else {
					fg_color = ppu->C64_to_rgb(bus->ppu_read(bus, VIC_COLOR_START + (row * 40 + col)));
					uint8_t char_code = bus->ppu_read(bus, ppu->screen_ram + (row * 40 + col));
					uint8_t pixel_data = bus->ppu_read(bus, ppu->char_ram + (char_code * 0x8) + (ppu->raster % 0x8));
					uint8_t bit_pos = (x - DXSTART) % 0x8;
					if ((bus->cpu_read(bus, CNTRL1) >> 0x6) & 0x1) {
						uint8_t bg_index = (char_code >> 0x6) & 0x3;
						t_bg_color = ppu->C64_to_rgb(bus->cpu_read(bus, BACKG_COLOR2 + bg_index));
					}
					else	t_bg_color = bg_color;
					mlx_put_pixel(ppu->mlx_img, x, ppu->raster, (pixel_data & (0x80 >> bit_pos)) ? fg_color : t_bg_color);
				}
			}
		}
	}
	ppu->raster++;
	if (ppu->raster == ppu->get_raster(ppu)) {
		uint8_t _D019 = bus->cpu_read(bus, INTR_STATUS);
		bus->cpu_write(bus, INTR_STATUS, _D019 | 0x1);
		if ((bus->cpu_read(bus, INTR_ON) & 0x1) && !((_6502*)bus->cpu)->get_flag((_6502*)bus->cpu, 'I')) {
			((_6502*)bus->cpu)->irq_pending = 1;
		}
	}
	if (ppu->raster > GHEIGHT) 
		ppu->raster = 0;
	return 7;
}

uint8_t	IRQ_interrupt(_bus *bus, _6502 *mos6502) {
	if (mos6502->irq_pending) {
		printf("irq_interrupt\n");
		uint8_t irq_vector_low = mos6502->bus->cpu_read(mos6502->bus, IRQ_BRK);
		uint8_t irq_vector_high = mos6502->bus->cpu_read(mos6502->bus, IRQ_BRK + 1);
		uint8_t _D019 = bus->cpu_read(bus, INTR_STATUS);
		
		bus->cpu_write(bus, INTR_STATUS, _D019 | 0x1);

		mos6502->push(mos6502, mos6502->PC >> 8);
		mos6502->push(mos6502, mos6502->PC & 0x00FF);
		mos6502->set_flag(mos6502, 'B', 1);
		mos6502->push(mos6502, mos6502->SR);
		mos6502->set_flag(mos6502, 'I', 1);

		mos6502->PC = irq_vector_high << 8 | irq_vector_low;
		mos6502->irq_pending = 0;
		return 7;
	}
	return 0;
}

uint8_t	NMI_interrupt(_bus *bus, _6502 *mos6502) {
	if (mos6502->nmi_pending) {
		printf("nmi_interrupt\n");
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

void	handle_cia_timers(_bus *bus, _CIA *CIA) {
	(void)bus;
	(void)CIA;
}

// /// /	CYCLE	1ppu-step = 1cpu-step

void	*instruction_cycle(void *p) {
	_bus	*bus = (_bus*)p;
	_6502	*mos6502 = (_6502*)bus->cpu;
	_VIC_II	*ppu = (_VIC_II*)bus->ppu;

	while (1) {
		//	Sync
		pthread_mutex_lock(&bus->t_data->halt_mutex);
		if (bus->t_data->halt)
			break;
		pthread_mutex_unlock(&bus->t_data->halt_mutex);

		//	CIAs
		handle_cia_timers(bus, bus->cia1);
		handle_cia_timers(bus, bus->cia2);
	
		if (mos6502->cycles) {
			mos6502->cycles--;
			continue;
		}

		//	CPU
		mos6502->opcode = bus->cpu_read(bus, mos6502->PC);
		mos6502->cycles = mos6502->opcodes[mos6502->opcode](mos6502);

		//	PPU
		mos6502->cycles += ppu_step(ppu);
			// always 7 === 63 / 8(VIC cycles to CPU cycles)

		//	INTERRUPTS
		mos6502->cycles += NMI_interrupt(bus, mos6502); // 7 if set
		mos6502->cycles += IRQ_interrupt(bus, mos6502); // 7 if set
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
		printf("%04X  %02X %02X %02X  ",
			mos6502->PC, mos6502->opcode, bus->cpu_read(bus, mos6502->PC+1), bus->cpu_read(bus, mos6502->PC+2));
		printf("\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\t",
			mos6502->A, mos6502->X, mos6502->Y, mos6502->SR, mos6502->SP);

*/

