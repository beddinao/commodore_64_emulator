#include "metallc64.h"

void	cia_advance_timers(_bus *bus, _CIA *CIA, unsigned cycles) {
	uint32_t	new_timer_val;
	_cia_tod	*TOD = CIA->TOD;
	struct timeval cur_time;
	memset(&cur_time, 0, sizeof(cur_time));

	// Timer A
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
	// Timer B
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

	// TOD
	gettimeofday(&cur_time, NULL);
	unsigned secs_diff = (cur_time.tv_sec + cur_time.tv_usec*MICROS_TO_SECOND) -
		(TOD->time.tv_sec + TOD->time.tv_usec*MICROS_TO_SECOND);
	unsigned th_diff = secs_diff * 10;

	TOD->th_secs -= th_diff;
	if (TOD->th_secs <= 0) {
		TOD->secs -= abs(TOD->th_secs) / 10;
		TOD->th_secs = 10 - abs(TOD->th_secs) % 10;
		if (TOD->secs <= 0) {
			TOD->mins -= abs(TOD->secs) / 60;
			TOD->secs = 60 - abs(TOD->secs) % 60;
			if (TOD->mins <= 0) {
				TOD->hrs -= abs(TOD->mins) / 60;
				TOD->mins = 60 - abs(TOD->mins) % 60;
				if (TOD->hrs <= 0) {
					TOD->hrs = 12;
					TOD->PM = TOD->PM ? FALSE : TRUE;
				}
			}
		}
	}

	if (TOD->interrupt_enable
		&& TOD->th_secs == TOD->th_secs_alarm
		&& TOD->secs == TOD->secs_alarm
		&& TOD->mins == TOD->mins_alarm
		&& TOD->hrs == TOD->hrs_alarm
		&& !((_6502*)bus->cpu)->get_flag(bus->cpu, 'I')) {
		((_6502*)bus->cpu)->irq_pending = TRUE;
		TOD->interrupt_triggered = TRUE;
	}

	TOD->time.tv_sec = cur_time.tv_sec;
	TOD->time.tv_usec = cur_time.tv_usec;
}

void	cia_init(_CIA *cia, uint8_t addr, void* htod) {
	memset(cia, 0, sizeof(_CIA));
	_cia_tod *tod = (_cia_tod*)htod;
	cia->init = cia_init;
	cia->high_addr = addr;
	tod->th_secs = 10;
	tod->secs = 60;
	tod->mins = 60;
	tod->hrs = 12;
	tod->PM = 0; // AM
	tod->latched = 0;
	gettimeofday(&tod->time, NULL);
	cia->TOD = tod;
}

