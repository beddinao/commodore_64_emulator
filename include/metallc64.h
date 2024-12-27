#ifndef METALLC64_H
#define METALLC64_H

#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>
#include "MLX42.h"

/*
		ADDRESSABLE RANGE
*/
#define ADDR_RANGE		0x10000 // 64Kb - 6502/6510 addressable range
#define VIC_ADDR_RANGE	0x4000  // 16Kb - VIC-II addressable range
/*
		6510 CPU HIGH LEVEL MEMORY MAP 
*/
			/// Zero-page and stack
#define IO_PORT_DDR		0x0000  // data direction register
#define IO_PORT_DR		0x0001  // data register
#define ZP_START		0x0002  // Start of zero page usable memory
#define ZP_END		0x00FF  // end of zero page
#define STACK_ST		0x0100  // start of stack
#define STACK_END		0x01FF  // $0200 end of stack

			/// Basic
#define BASIC_RAM_START	0x0801  // BASIC program start
#define BASIC_ROM_START	0xA000  // BASIC ROM start
#define BASIC_ROM_END	0xBFFF  // BASIC ROM end
#define BASIC_ROM_SIZE	0x2000  // 8Kb
			        
			/// RAM area
#define UPP_RAM_START	0xC000  // Upper RAM start
#define UPP_RAM_END		0xCFFF  // Upper RAM end

			/// I/O and Character ROM
#define IO_CHAR_START	0xD000  // start of IO/Char-ROM
#define IO_CHAR_END		0xDFFF  // END of IO/Char-ROM

			/// VIC-II Registers
#define VIC_REG_START	0xD000  // VIC-II registers start
#define VIC_REG_END		0xD3FF  // VIC-II registers end
#define VIC_COLOR_START	0xD800  // Color RAM start
#define VIC_COLOR_END	0xDBFF  // Color RAM end

			/// SID - Sound Interface Device
#define SID_REG_START	0xD400  // SID registers start
#define SID_REG_END		0xD7FF  // SID registers end

			/// CIA - Complex Interface Adapters
#define CIA1_START		0xDC00  // CIA #1 registers start
#define CIA1_END		0xDCFF  // CIA #1 registers end
#define CIA2_START		0xDD00  // CIA #2 registers start
#define CIA2_END		0xDDFF  // CIA #2 registers end

			/// KERNAL ROM
#define KERNAL_ROM_START	0xE000  // Kernal ROM start
#define KERNAL_ROM_END	0xFFFF  // Kernal ROM end
#define KERNAL_ROM_SIZE	0x2000  // 8Kb

			/// INTERRUPT VECTORS
#define NMI		0xFFFA  // $FFFA-$FFFB non-maskable interrupt
#define RSTV		0xFFFC  // $FFFC-$FFFD reset vector
#define IRQ_BRK		0xFFFE  // $FFFE-$FFFF interrupt req/brk
/*
		VIC-II HIGH LEVEL MEMORY MAP
		16KB bank at a time
*/
#define VIC_BANK_SIZE	0x4000
			/// VIC BANKS, by CIA2 bits 0-1
#define VIC_BANK_3		0x0000  // $0000-$3FFF
#define VIC_BANK_2		0x4000  // $4000-$7FFF
#define VIC_BANK_1		0x8000  // $8000-$BFFF
#define VIC_BANK_0		0xC000  // $C000-$FFFF

			/// CHARACTER MEMORY
#define LOW_CHAR_ROM_START	0x1000  // first Char-ROM start
#define UPP_CHAR_ROM_START	0x9000  // second Char-ROM start
#define CHAR_ROM_SIZE	0x1000  // 4Kb

			/// SCREEN MEMORY
#define DEFAULT_SCREEN	0x0400  // default screen-memory location
#define SCREEN_SIZE		0x0400  // 1Kb
/*
		ROMS PATHS
*/
#define KERNAL_PATH		"./assets/roms/kernal.901227-03.bin"
#define BASIC_PATH		"./assets/roms/basic.901226-01.bin"
#define CHAR_ROM_PATH	"./assets/roms/characters.901225-01.bin"
/*
		KBYTES UNITS in BYTES
*/
#define _64KB		0x10000 // 65536
#define _32KB		0x8000  // 32768
#define _16KB		0x4000  // 16384
#define _08KB		0x2000  // 8192
#define _04KB		0x1000  // 4096
/*
		DISPLAY DIMENSIONS
*/
#define DPPWP		1       // display pixels per window pixel
#define WIN_HEIGHT		240     // mlx42 window height
#define WIN_WIDTH		256     // mlx42 window width
#define MIN_HEIGHT		100     // window min height
#define MIN_WIDTH		100     // window min width
/*
		ANSI CODES
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
	uint8_t		RAM[ADDR_RANGE];
	//
	void		(*cpu_write)(struct _bus*, uint16_t, uint8_t);
	uint8_t		(*cpu_read)(struct _bus*, uint16_t);
	void		(*ppu_write)(struct _bus*, uint16_t, uint8_t);
	uint8_t		(*ppu_read)(struct _bus*, uint16_t);
	uint8_t		(*load_basic)(struct _bus*);
	uint8_t		(*load_kernal)(struct _bus*);
	uint8_t		(*load_chars)(struct _bus*);
	void		(*reset)(struct _bus*);
	//
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

typedef	struct VIC_II {

	mlx_t		*mlx_ptr; // MLX42 window
	mlx_image_t	*mlx_img;
	unsigned		win_width;
	unsigned		win_height;

	_bus		*bus;	// BUS Address
}	_VIC_II;

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
void	draw_bg(_VIC_II*, unsigned);
void	draw_line(_VIC_II*, int, int, int, int, int);

#endif
