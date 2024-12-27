#ifndef METALLC64_H
#define METALLC64_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>
#include "MLX42.h"

/*
		CPU ADDRESSABLE RANGE
*/
//	RAM ARRAY START
#define ADDR_RANGE		0x10000 // 64Kb - 6502 addressable range

#define RAM_START		0x0000  // 2Kb of ram
#define RAM_SIZE		0x0800

#define STACK_START		0x0100  // 256
#define STACK_END		0x0200  // 512
//	RAM ARRAY END	$0800
//	CARTRIDGE ARRAY START
#define RAM_MIRR_ST		0x0000  // addr_rng:$800: mirrored to ram
#define RAM_MIRR_EN		0x17FF  // addr_rng:$1FFF

			//// PPU REGISTERS
#define PPUCTRL		0x1800  // addr_rng:$2000: control
#define PPUMASK		0x1801  // addr_rng:$2001: mask
#define PPUSTATUS		0x1802  // addr_rng:$2002: status
#define OAMADDR		0x1803  // addr_rng:$2003: OAM address
#define OAMDATA		0x1804  // addr_rng:$2004: OAM data
#define PPUSCROLL		0x1805  // addr_rng:$2005: scroll
#define PPUADDR		0x1806  // addr_rng:$2006: VRAM address
#define PPUDATA		0x1807  // addr_rng:$2007: VRAM data

#define MIRR_PPU_REGS	0x1808  // addr_rng:$2008: mirrored to PPU registers
#define MIRR_PPU_REGS_EN	0x37FF  // addr_rng:$3FFF

#define APU_IO_REGS		0x3800  // addr_rng:$4000: APU and io registers
#define APU_IO_REGS_EN	0x381F  // addr_rng:$401F

#define PRG_RAM		0x5800  // addr_rng:$6000: extra ram
#define PRG_RAM_EN		0x77FF  // addr_rng:$7FFF

#define PRG_ROM		0x7800  // addr_rng:$8000: program start
#define PRG_ROM_EN		0xF7FF  // addr_rng:$FFFF
#define PRGM_MSIZE		0x8000  // 32kb

			////	INTERRUPT VECTORS
#define NMI		0xF7FA  // addr_rng:$FFFA-$FFFB non-maskable interrupt
#define _16RSTV		0xB7FC  // addr_rng:$FFFC-$FFFD reset vector for one bank
#define _32RSTV		0xF7FC  // addr_rng:$FFFC-$FFFD reset vector for multible banks
#define IRQ_BRK		0xF7FE  // addr_rng:$FFFE-$FFFF interrupt req/brk
//	CARTRIDGE ARRAY END	$F7FF
/*
		PPU ADDRESSABLE RANGE
*/
//	VRAM ARRAY START
#define VRAM_SIZE		0x3FFF  // ppu address range 16383
#define PTRN_TAB		0x0000  // -> $1FFF, 0->8191, 8191 of vram: pattern table
#define NME_TAB		0x2000  // -> $3EFF, 8192->16127, 7935 of vram: nametable
#define PLT_MEM		0x3F00  // -> $3FFF, 16128->16383, 256 of vram: palette memory
//	VRAM ARRAY END	$3FFF
/*
	PATTERN TABLES VRAM($0000-$1FFF) 4095 each
*/
#define PTRN_TAB_0		0x0000  // - $0FFF
#define PTRN_TAB_1		0x1000  // - $1FFF
/*
	NAMETABLES VRAM($2000-$3EFF) 960 Bytes each, 32x30tiles 8Bytes each
*/
#define NMETAB_0		0x2000  // - $23FF
#define NMETAB_1		0x2400  // - $27FF
#define NMETAB_2		0x2800  // - $2BFF
#define NMETAB_3		0x2C00  // - $2FFF
/*
		BOTH ADDRESSABLE RANGES
*/
//	CHR_RAM ARRAY START
#define PPU_CHR_RAM		0x0000
#define CPU_CHR_RAM		0x0000
#define CHR_RAM_SIZE	0x2FFF
//	CHR_RAM ARRAY END	$2FFF
/*
		KBytes UNITS in Bytes
*/
#define _64KB		0x10000 // 65536
#define _32KB		0x8000  // 32768
#define _16KB		0x4000  // 16384
#define _08KB		0x2000  // 8192
#define _04KB		0x1000  // 4096
/*
		DIMENTIONS
*/
//	DISPLAY
#define PPT		8       // pixels per tile
#define P_WIDTH		256     // width in pixels
#define P_HEIGHT		240
#define T_WIDTH		32      // width in tiles
#define T_HEIGHT		30
//	MLX WINDOW
#define DPPWP		1       // display pixels per window pixel
#define WIN_HEIGHT		240
#define WIN_WIDTH		256
#define MIN_HEIGHT		100
#define MIN_WIDTH		100
/*
		ANSI codes
*/
#define RST		"\x1B[0m"
#define RED		"\x1B[31m"
#define BLU		"\x1B[34m"
#define WHT		"\x1B[37m"
#define UND		"\033[4m"

typedef	struct thread_data {
	pthread_t		worker;
	pthread_mutex_t	halt_mutex;
	pthread_mutex_t	data_mutex;
	uint8_t		halt;
	void		*ppu;	// \ for access inside
	void		*cpu;	// /  signal handlers
}	thread_data;

typedef	struct _bus {
	uint8_t		ram[RAM_SIZE];
	uint8_t		cartridge[ADDR_RANGE-RAM_SIZE];
	uint8_t		vram[VRAM_SIZE];
	uint8_t		chr_ram[CHR_RAM_SIZE];  // if no CHR-ROM
	//
	void		(*cpu_write)(struct _bus*, uint16_t, uint8_t);
	uint8_t		(*cpu_read)(struct _bus*, uint16_t);
	void		(*ppu_write)(struct _bus*, uint16_t, uint8_t);
	uint8_t		(*ppu_read)(struct _bus*, uint16_t);
	uint8_t		(*load_ROM)(struct _bus*, char*);
	void		(*reset)(struct _bus*);
	//
	unsigned		rstv;	// dynamic reset vector
	void		*ppu;
	void		*cpu;
	thread_data	*t_data;
}	_bus;

typedef	struct _6502 { 
	unsigned		PC;	// program counter (16bits)
	uint8_t		A;	// Accumulator
	uint8_t		X;	// Index register
	uint8_t		Y;	// Index register
	uint8_t		SP;	// stack pointer
	uint8_t		SR;	// status register
				// N V - B D I Z C
	uint8_t		(*opcodes[0x100])(struct _6502*);
				// FxF opcodes matrix
	uint8_t		(*pull)(struct _6502*);
	void		(*push)(struct _6502*, uint8_t);
	void		(*set_flag)(struct _6502*, uint8_t, uint8_t);
	uint8_t		(*get_flag)(struct _6502*, uint8_t);
	void		(*reset)(struct _6502*);
	void		*(*instruction_cycle)(void*);
				// fetch-decode-execute
	uint8_t		opcode;	// last fetched opcode
	uint8_t		cycles;   // last instr. cycles count
	uint8_t		nmi_pending;
				// NMI interrupt flag
	_bus		*bus;	// BUS Address
}	_6502;

typedef	struct ppu {
	uint8_t		ctrl;     // $2000
	uint8_t		mask;	// $2001
	uint8_t		status;	// $2002
	uint8_t		oamaddr;	// $2003
	uint8_t		oamdata;	// $2004
	uint8_t		scroll;	// $2005
	uint8_t		addr;	// $2006
	uint8_t		data;	// $2007

	mlx_t		*mlx_ptr; // MLX42 window
	mlx_image_t	*mlx_img;
	unsigned		win_width;
	unsigned		win_height;
	uint32_t		screen[P_WIDTH * P_HEIGHT];
				// RGB values for display
	_bus		*bus;	// BUS Address
	uint8_t		vram_addr_latch;
	uint8_t		latch_first_byte;
				// PPUADDR ($2006) latch
	unsigned		scanline;
	unsigned		cycle;
}	_ppu;

/* cycle.c */
void	*instruction_cycle(void*);

/* instructions.c */
void	load_instructions(_6502*);

/* cpu_methods.c */
void	cpu_init(_6502*);

/* bus.c */
void	bus_init(_bus*);

/* hooks.c */
void	setup_mlx_hooks(void*);

/* ppu.c */
void	loop_hook(void*);

/* main.c */
void	sig_handle(int);

/* draw_utils.c */
void	draw_bg(_ppu*, unsigned);
void	draw_line(_ppu*, int, int, int, int, int);
void	render_screen(_ppu*);

#endif
