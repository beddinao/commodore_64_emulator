#include "metallc64.h"

void	cia_advance_timers(_bus *bus, _CIA *CIA, unsigned cycles) {
	uint32_t	new_timer_val;
	if (CIA->TA_enable && !CIA->TA_input_mode) {
		new_timer_val = CIA->timerA - cycles;
		if (new_timer_val > CIA->timerA) {
			if (CIA->TA_mode) 
				CIA->timerA = CIA->TA_latch_high << 0x8 | CIA->TA_latch_low;
			else 	CIA->TA_enable = FALSE;

			if (CIA->TA_interrupt_enable) {
				if (CIA->high_addr == 0xDC && !((_6502*)bus->cpu)->get_flag(bus->cpu, 'I')) {
					((_6502*)bus->cpu)->irq_pending = TRUE;
					CIA->TA_interrupt_triggered = TRUE;
				}
				else if (CIA->high_addr == 0xDD) {
					((_6502*)bus->cpu)->nmi_pending = TRUE;
					CIA->TA_interrupt_triggered = TRUE;
				}
			}

		}
		else	CIA->timerA = new_timer_val;
	}
	if (CIA->TB_enable && CIA->TB_input_mode) {
		new_timer_val = CIA->timerB - cycles;
		if (new_timer_val > CIA->timerB) {
			if (CIA->TB_mode)
				CIA->timerB = CIA->TB_latch_high << 0x8 | CIA->TB_latch_low;
			else	CIA->TB_enable = FALSE;

			if (CIA->TB_interrupt_enable) {
				if (CIA->high_addr == 0xDC && !((_6502*)bus->cpu)->get_flag(bus->cpu, 'I')) {
					((_6502*)bus->cpu)->irq_pending = TRUE;
					CIA->TB_interrupt_triggered = TRUE;
				}
				else if (CIA->high_addr == 0xDD) {
					((_6502*)bus->cpu)->nmi_pending = TRUE;
					CIA->TB_interrupt_triggered = TRUE;
				}
			}
		}
		else	CIA->timerB = new_timer_val;
	}
}

void	cia_init(_CIA *cia, uint8_t addr) {
	memset(cia, 0, sizeof(_CIA));
	cia->init = cia_init;
	cia->high_addr = addr;
}

