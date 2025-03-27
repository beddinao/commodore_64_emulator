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
#include <ctype.h>
#include <limits.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <SDL3/SDL.h> 

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
#define BASIC_PRG_START	0x0801  // BASIC program start
#define BASIC_PRG_END	0x9FFF  // BASIC program end
#define BASIC_PRG_SIZE	0x97FE  // BASIC program size
#define BASIC_ROM_START	0xA000  // BASIC ROM start
#define BASIC_ROM_END	0xBFFF  // BASIC ROM end
#define BASIC_ROM_SIZE	0x2000  // 8Kb
			        
			/// RAM area
#define UPP_RAM_START	0xC000  // Upper RAM start
#define UPP_RAM_END		0xCFFF  // Upper RAM end

			/// I/O and Character ROM
#define IO_CHAR_START	0xD000  // start of IO/Char-ROM
#define IO_CHAR_END		0xDFFF  // END of IO/Char-ROM

			/// VIC-II Registers range
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
#define VIC_BANK_0		0x0000  // $0000-$3FFF
#define VIC_BANK_1		0x0001  // $4000-$7FFF
#define VIC_BANK_2		0x0002  // $8000-$BFFF
#define VIC_BANK_3		0x0003  // $C000-$FFFF

			/// CHARACTER MEMORY
#define LOW_CHAR_ROM_START	0x1000  // first Char-ROM start
#define UPP_CHAR_ROM_START	0x9000  // second Char-ROM start
#define CHAR_ROM_SIZE	0x1000  // 4Kb

			/// SCREEN MEMORY
#define DEFAULT_SCREEN	0x0400  // default screen-memory location
#define SCREEN_SIZE		0x0400  // 1Kb
/*
		VIC-II ALL REGISTERS
		$D000 - $D3FF
*/
#define SPRITE0_X		0xD000  // Sprite #0 X
#define SPRITE0_Y		0xD001  // Sprite #0 Y
#define SPRITE1_X		0xD002  // Sprite #1 X
#define SPRITE1_Y		0xD003  // Sprite #1 Y
#define SPRITE2_X		0xD004  // Sprite #2 X
#define SPRITE2_Y		0xD005  // Sprite #2 Y
#define SPRITE3_X		0xD006  // Sprite #3 X
#define SPRITE3_Y		0xD007  // Sprite #3 Y
#define SPRITE4_X		0xD008  // Sprite #4 X
#define SPRITE4_Y		0xD009  // Sprite #4 Y
#define SPRITE5_X		0xD00A  // Sprite #5 X
#define SPRITE5_Y		0xD00B  // Sprite #5 Y
#define SPRITE6_X		0xD00C  // Sprite #6 X
#define SPRITE6_Y		0xD00D  // Sprite #6 Y
#define SPRITE7_X		0xD00E  // Sprite #7 X
#define SPRITE7_Y		0xD00F  // Sprite #7 Y
#define MSBS_X		0xD010  // Sprite #0-#7 Xs (bit #8)
#define CNTRL1		0xD011  // Control Register 1
#define RASTER		0xD012  // Raster(Scanline) Counter
#define LIGHT_PEN_X		0xD013  // Light-Pen X-Coordinate
#define LIGHT_PEN_Y		0xD014  // Light-Pen Y-Coordinate
#define SPRITES_ON		0xD015  // Sprite Enabled (bit #x == sprite #x)
#define CNTRL2		0xD016  // Control Register 2
#define SPRITE_EXP_Y	0xD017  // Sprite Y Expansion
#define MEM_SETUP		0xD018  // Memory Setup (pointers)
#define INTR_STATUS		0xD019  // Interrupt status register
#define INTR_ON		0xD01A  // Interrupt control register
#define SPRITE_PRT		0xD01B  // Sprite data priority
#define SPRITE_MULTI_COL	0xD01C  // Sprite multicolor register
#define SPRITE_EXP_X	0xD01D  // Sprite X Expansion
#define SPR_SPR_COLL	0xD01E  // Sprite-Sprite Collision
#define SPR_BACK_COLL	0xD01F  // Sprite-Background Collision
#define BRD_COLOR		0xD020  // Border Color
#define BACKG_COLOR0	0xD021  // Background Color 0
#define BACKG_COLOR1	0xD022  // Background Color 1
#define BACKG_COLOR2	0xD023  // Background Color 2
#define BACKG_COLOR3	0xD024  // Background Color 3
#define SPR_MULTI_COL0	0xD025  // Sprite Multicolor 0
#define SPR_MULTI_COL1	0xD026  // Sprite Multicolor 1
#define SPR_CLR_0		0xD027  // Color Sprite 0
#define SPR_CLR_1		0xD028  // Color Sprite 1
#define SPR_CLR_2		0xD029  // Color Sprite 2
#define SPR_CLR_3		0xD02A  // Color Sprite 3
#define SPR_CLR_4		0xD02B  // Color Sprite 4
#define SPR_CLR_5		0xD02C  // Color Sprite 5
#define SPR_CLR_6		0xD02D  // Color Sprite 6
#define SPR_CLR_7		0xD02E  // Color Sprite 7
/*
		$D02F-$D03F 17Bytes: UNUSABLE
		$D040-$D3FF VIC-II REGISTER IMAGES TODO
*/
/*
		CIA1/CIA2 ALL REGISTERS
		$DC00-$DCFF/$DD00-$DDFF
		ONLY THE LOW BYTE AS THIS MAP COVERS TWO REGIONS
		$DCxx / $DDxx
*/
#define KEY_PORTA		0x00   // Port A Data Direction
#define KEY_PORTB		0x01   // Port B Data Direction
#define DDR_PORTA		0x02   // Port A Data Register
#define DDR_PORTB		0x03   // Port B Data Register
#define TIMERA_LOW		0x04   // Timer A Low Byte
#define TIMERA_HIGH		0x05   // Timer A High Byte
#define TIMERB_LOW		0x06   // Timer B Low Byte
#define TIMERB_HIGH		0x07   // Timer B High Byte
#define TOD_TSECS		0x08   // time-of-day 10th seconds
#define TOD_SECS		0x09   // time-of-day seconds
#define TOD_MINS		0x0A   // time-of-day minutes
#define TOD_HRS		0x0B   // time-of-day hours
#define SSR		0x0C   // Serial Data Register
#define CIA_CNTRL		0x0D   // Interrupt Control register
#define TIMERA_CNTRL	0x0E   // Timer A control
#define TIMERB_CNTRL	0x0F   // Timer B control
/*
		SPRITES
		higher region of screen ram
*/
#define SPRT_H		21
#define SPRT_W		24
#define SPRT_PTRS_ADDR	0x03F8 // relative to screen ram
#define SPRT_PTRS_SIZE	0x8    // bytes
/*
		TIMING
		PAL system Configurations
*/
#define CYCLES_PER_SECOND	985248
#define PAL_FPS		50
#define VIC_CYCLES_PER_LINE	63
#define MICROS_TO_SECOND	1000000
#define NANOS_TO_SECOND	1000000000
#define CYCLES_PER_FRAME	CYCLES_PER_SECOND / PAL_FPS
#define MICROS_PER_FRAME	MICROS_TO_SECOND / PAL_FPS
#define NANOS_PER_FRAME	NANOS_TO_SECOND / PAL_FPS
/*
		ROMS PATHS
*/
#define KERNAL_PATH		"./assets/roms/kernal.901227-03.bin"
#define BASIC_PATH		"./assets/roms/basic.901226-01.bin"
#define CHAR_ROM_PATH	"./assets/roms/characters.901225-01.bin"
/*
		D64 DISK IMAGE
*/
#define EXTRACTED_PRGS	"./programs/generated/"
#define SECTOR_SIZE		256
#define DIR_TRACK		18
#define DIR_SECTOR		1
#define SECTORS_PER_TRACK	21      // tracks 1-17
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
#define GWIDTH		504     // window width
#define GHEIGHT		312     // window height
			        // in PAL System (Phase Alternating Line)
#define PWIDTH		320     // display window 45 Chars, 8pixels each
#define PHEIGHT		200     // display window 40 chars, 8pixels each
#define CWIDTH		40      // characters per line
#define CHEIGHT		25      // lines per frame

#define DXSTART		(GWIDTH-PWIDTH)/2
#define DXEND		GWIDTH-DXSTART     
#define DYSTART		(GHEIGHT-PHEIGHT)/2
#define DYEND		GHEIGHT-DYSTART
/*
		WINDOW DIMENSIONS
*/
#define WPDX		2       // x window pixels per display
#define WPDY		2       // y window per display
#define WHEIGHT		GHEIGHT*WPDY
#define WWIDTH		GWIDTH*WPDX
#define MHEIGHT		GHEIGHT // window min height
#define MWIDTH		GWIDTH  // window min width
#define HTOW		GWIDTH / GHEIGHT
/*
		ANSI CODES
*/
#define RST		"\x1B[0m"
#define RED		"\x1B[31m"
#define BLU		"\x1B[34m"
#define CYN		"\x1B[36m"
#define WHT		"\x1B[37m"
#define GRN		"\x1B[32m"
#define YEL		"\x1B[33m"
#define ORG		"\e[0;91m"
#define UND		"\033[4m"

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

#define SHELL_PRMPT		"\x1B[37mC64Shell\x1B[0m$ "
#define PATH_MAX_SIZE	0x400

typedef	struct thread_data {
	pthread_t		worker;   // => shell interface 
	pthread_mutex_t	halt_mutex;
	pthread_mutex_t	prg_mutex;
	pthread_mutex_t	cmd_mutex;
	bool		halt;     // <- halt_mutex
	bool		load;     // <-|prg_mutex
	bool		reset;    // <-|
	bool		cmd;      // <- cmd_mutex

	char		*line;    // readline allocated line
			          // for freeing later
	void		*bus;
}	thread_data;

typedef	struct _bus {
	uint8_t		RAM[ADDR_RANGE];
	uint8_t		KERNAL[KERNAL_ROM_SIZE];
	uint8_t		BASIC[BASIC_ROM_SIZE];
	uint8_t		CHARACTERS[CHAR_ROM_SIZE];
	//
	void		(*cpu_write)(struct _bus*, uint16_t, uint8_t);
	uint8_t		(*cpu_read)(struct _bus*, uint16_t);
	uint8_t		(*ram_read)(struct _bus*, uint16_t);
	void		(*ram_write)(struct _bus*, uint16_t, uint8_t);
	uint8_t		(*load_roms)(struct _bus*);
	struct _bus	*(*reset)();
	void		(*clean)(struct _bus*);
	//
	void		*cpu;
	void		*vic;
	void		*cia1;
	void		*cia2;
	void		*prg;
	void		*cmd;
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
	struct _6502	*(*reset)(_bus*);
	void		*(*instruction_cycle)(void*);
				// fetch-decode-execute
	uint8_t		opcode;	// last fetched opcode
	unsigned		cycles;   // cpu cycles counter
				// and global timing sync
	bool		nmi_pending;
	bool		irq_pending;
				// interrupt flags
	_bus		*bus;	// BUS Address
}	_6502;

typedef	struct VIC_II {
	uint16_t		(*get_raster)(struct VIC_II*);
	void		(*increment_raster)(struct VIC_II*, uint16_t);
	uint32_t		(*C64_to_rgb)(uint8_t);
	struct VIC_II	*(*init)(_bus*);

	uint8_t		*vic_memory[4];

	SDL_Window	*win; // SDL window 
	SDL_Renderer	*renderer;
	unsigned		win_width;
	unsigned		win_height;
	float		wpdx;
	float		wpdy;	// window pixels per display
				// pixels, X/Y

	uint16_t		raster;   // dynamic raster counter
	bool		raster_interrupt_enable;
	bool		raster_interrupt_triggered;

	uint16_t		screen_ram;
				// dynamic screen ram
	uint16_t		bank;	// current bank address
	uint16_t		bitmap_ram;
	uint16_t		char_ram; // dynamic character ram
	bool		char_rom_on;
				// $D018 bits#1#3 values %010 | %011
				// in bank0 | bank2
				// sets this flag to fetch ROM not RAM

	unsigned		cycles;	// vic independent
				// cycles counter
	uint8_t		sprite_enable;
				// $D015, bit#x = sprite#x
	uint16_t		sprite_ptrs;
	uint8_t		sprite_x[8];
	uint8_t		sprite_y[8];
	uint8_t		sprite_8x;
				// $D000-$D010
				// x:%2, y:%3
	uint8_t		sprite_colors[8];
				// $D017-$D02E
				// bits#0-#3
	uint8_t		sprite_multicolor_enable;
	uint8_t		sprite_multicolor0;
	uint8_t		sprite_multicolor1;
				// bit#x on/off$D01C
				// 0:$D025, 1:$D026
	uint8_t		sprite_expand_x;
	uint8_t		sprite_expand_y;
				// double w:$D01D, h:$D017
	uint8_t		sprite_priority;
				// $D01B
				// bit#x 0:front 1:back
	bool		sp_sp_interrupt_enable;
	bool		sp_bg_interrupt_enable;
	bool		sp_sp_interrupt_triggered;
	bool		sp_bg_interrupt_triggered;

	uint8_t		sp_sp_collision;
	uint8_t		sp_bg_collision;

	uint8_t		control1;
	uint8_t		control2;
	_bus		*bus;	// BUS Address
}	_VIC_II;

typedef	struct CIA {
	uint8_t		high_addr;// CIA each ship address
				// $DC = CIA#1 / $DD = CIA#2
	uint16_t		timerA;
	uint16_t		timerB;  // 16bit timers
			         // combination of latches below
	uint8_t		TA_latch_low;
	uint8_t		TA_latch_high;
	uint8_t		TB_latch_low;
	uint8_t		TB_latch_high;
			        // timers low/high bytes
	bool		TA_enable;
	bool		TB_enable;
	bool		TA_interrupt_enable;
	bool		TB_interrupt_enable;
			         // timers states
			         // and if they can set
			         // interrupts
	bool		TA_mode; // 0: restart, 1: one-shot
	bool		TB_mode;
	bool		TA_input_mode;
	bool		TB_input_mode;
			         // timer counts eather
			         // 0: cycles, 1: CNT pin
	bool		TA_interrupt_triggered;
	bool		TB_interrupt_triggered;
			         // NMI/IRQ triggered
			         // by underflow
	struct CIA	*(*init)(_bus*, uint8_t);
	void		(*clean)(struct CIA*);
	void		*keys;
	void		*TOD;
}	_CIA;

typedef	struct keymap {
	uint8_t		matrix[0x8];
	uint8_t		active_row;   // current row being scanned 
			    // $DC00, 11111011 -> row 2
/*
          C64 PORTS A/B KEYS LAYOUT

                           column = $DC01
          |bit#0|bit#1|bit#2|bit#3|bit#4|bit#5|bit#6|bit#7|
           -----------------------------------------------
       #0 | DEL | RET | CL  | F7  | F1  | F3  | F5  |CR_UD|
       #1 |  3  |  W  |  A  |  4  |  Z  |  S  |  E  |SFT_L|
row    #2 |  5  |  R  |  D  |  6  |  C  |  F  |  T  |  X  |
 =     #3 |  7  |  Y  |  G  |  8  |  B  |  H  |  U  |  V  |
$DC00  #4 |  9  |  I  |  J  |  0  |  M  |  K  |  O  |  N  |
       #5 |  +  |  P  |  L  |  -  |  .  |  :  |  @  |  ,  |
       #6 |  £  |  *  |  ;  |CR_HM|SFT_R|  =  |AR_UP|  /  |
       #7 |  1  |AR_LF|CNTRL|  2  |SPACE|COMDR|  Q  |RN/ST|



*/	
}	_keymap;

typedef	struct cia_clock {
	struct timeval 	time;
	int8_t		th_secs;
	int8_t		secs;
	int8_t		mins;
	int8_t		hrs;
	bool		PM; // 0 == AM
		    	//
	bool		latched;
	int8_t		th_secs_latch;
	int8_t		secs_latch;
	int8_t		mins_latch;

	bool		interrupt_enable;
	bool		write_mode; // 0 => sets TOD values
				  // 1 => sets alarm time
	bool		interrupt_triggered;
	
	uint8_t		th_secs_alarm;
	uint8_t		secs_alarm;
	uint8_t		mins_alarm;
	uint8_t		hrs_alarm;
}	_cia_tod;

typedef	struct basic_prg {
	bool		loaded;
	char		path[PATH_MAX_SIZE];
	char		buffer[BASIC_PRG_SIZE];
	uint16_t		ld_addr;
	uint16_t		en_addr;
	uint16_t		size;
}	_prg;

typedef	struct cmd {
	bool		done;
	char		cmd[0x4];
	uint16_t		st_addr;
	uint16_t		en_addr;
	unsigned		col;
}	_cmd;

/* cycle.c */
void	main_cycle(void*);

/* instructions.c */
void	load_instructions(_6502*);

/* cpu.c */
_6502	*cpu_init(_bus*);

/* bus.c */
_bus	*bus_init();

/* hooks.c */
void	key_event_handle(_bus*, SDL_Event*, bool);
void	window_event_handle(_bus*);

/* ppu.c */
void	loop_hook(void*);

/* main.c */
void	exit_handle(int);

/* vic.c */
uint32_t	C64_to_rgb(uint8_t);
void	vic_advance_raster(_bus*, _VIC_II*, unsigned);
_VIC_II	*vic_init(_bus*);

/* cia.c */
void	cia_advance_timers(_bus*, _CIA*, unsigned);
_CIA	*cia_init(_bus*, uint8_t);

/* draw_utils.c */
void	draw_bg(_VIC_II*, uint32_t);
void	draw_line(_VIC_II*, int, int, int, int, int);
void	put_raster(_VIC_II *, unsigned, unsigned, unsigned, uint32_t);
void	put_pixel(_VIC_II *, unsigned, unsigned, uint32_t);
SDL_Window	*init_window(_bus*, _VIC_II*);

/* shell.c */
void	*open_shell(void*);

/* loader.c */
void	prg_load_sequence(_bus*, _prg*);
void	reset_prg(_bus*, _prg*);
void	change_col(_bus*, _cmd*);

/* d64.c */
FILE*	read_d64file(FILE*, char*);

/* print.c */
void	print_memory(_bus*, _cmd*);
void	print_help(char*);
void	print_col_help(char*);

#endif
