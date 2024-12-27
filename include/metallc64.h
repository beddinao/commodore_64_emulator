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
			    
			////	INTERRUPT VECTORS
#define NMI		0xF7FA  // addr_rng:$FFFA-$FFFB non-maskable interrupt
#define _16RSTV		0xB7FC  // addr_rng:$FFFC-$FFFD reset vector for one bank
#define _32RSTV		0xF7FC  // addr_rng:$FFFC-$FFFD reset vector for multible banks
#define IRQ_BRK		0xF7FE  // addr_rng:$FFFE-$FFFF interrupt req/brk
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
