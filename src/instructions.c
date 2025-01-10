#include "metallc64.h"

// /// /	INSTRUCTIONS	

/*
	BRK - op0x00
	IMPLIED
	1 Byte, 7 Cycles
*/
uint8_t	BRK_IMP(_6502* mos6502) {
	//printf("BRK IMP");
	uint16_t brk_ker_addr = IRQ_BRK - KERNAL_ROM_START;
	/*uint8_t	brk_vector_low = mos6502->bus->cpu_read(mos6502->bus, IRQ_BRK);
	uint8_t	brk_vector_high = mos6502->bus->cpu_read(mos6502->bus, IRQ_BRK + 1);*/
	uint8_t	brk_vector_low = mos6502->bus->KERNAL[brk_ker_addr];
	uint8_t	brk_vector_high = mos6502->bus->KERNAL[brk_ker_addr+1];

	mos6502->PC++;
	mos6502->push(mos6502, ((mos6502->PC+1) >> 8) & 0xFF);
	mos6502->push(mos6502, (mos6502->PC+1) & 0xFF);

	mos6502->set_flag(mos6502, 'B', 1);
	mos6502->push(mos6502, mos6502->SR);
	mos6502->set_flag(mos6502, 'I', 1);

	// software interrupt is software's responsiblity
	if (!brk_vector_low && !brk_vector_high) {
		////printf("invalid interrupt address(0x%04X) skipping..", intr_addr);
		return 0;
	}

	mos6502->PC = (brk_vector_high << 8) | brk_vector_low;
	return 7;
}

/*
	ORA - op0x01
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	ORA_INDX(_6502* mos6502) {
	//printf("ORA_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->A |= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 6;
}


/*
	ORA - op0x05
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	ORA_ZP(_6502* mos6502){
	//printf("ORA_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	mos6502->A |= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	ASL - op0x06
	ZERO PAGE
	2 Bytes, 5 Cycles
*/
uint8_t	ASL_ZP(_6502* mos6502) {
	//printf("ASL_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	mos6502->set_flag(mos6502, 'C', (operand >> 0x7) & 0x1);
	operand <<= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, 0x00 << 0x8 | low_byte, operand);
	mos6502->PC += 2;
	return 5;
}

/*
	PHP - op0x08
	IMPLIED
	1 Byte, 3 Cycles
*/
uint8_t	PHP_IMP(_6502 *mos6502) {
	//printf("PHP_IMP");
	uint8_t pushed_status = mos6502->SR;
	pushed_status |= 0x20;
	pushed_status |= 0x10;
	/*mos6502->set_flag(mos6502, 'B', 1);
	mos6502->set_flag(mos6502, 'A', 1);*/
	mos6502->push(mos6502, pushed_status);
	mos6502->PC += 1;
	return 3;
}

/*
	ORA - op0x09
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	ORA_IMM(_6502 *mos6502) {
	//printf("ORA_IMM");
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->A |= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	ASL - op0x0A
	ACCUMULATOR
	1 Byte, 2 Cycles
*/
uint8_t	ASL_ACC(_6502 *mos6502) {
	//printf("ASL_ACC");
	mos6502->set_flag(mos6502, 'C', (mos6502->A >> 0x7) & 0x1);
	mos6502->A <<= 0x1;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', (mos6502->A >> 0x7) & 0x1);
	mos6502->PC += 1;
	return 2;
}

/*
	ORA - op0x0D
	ABSOLUTE
	3 Byte, 4 Cycles
*/
uint8_t	ORA_ABS(_6502 *mos6502) {
	//printf("ORA_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->A |= mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 0x3;
	return 4;
}

/*
	ASL - op0x0E
	ABSOLUTE
	3 Bytes, 6 Cycles
*/
uint8_t	ASL_ABS(_6502* mos6502) {
	//printf("ASL_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->set_flag(mos6502, 'C', (operand >> 0x7) & 0x1);
	operand <<= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, operand);
	mos6502->PC += 3;
	return 6;
}

/*
	BPL - op0x10
	RELATIVE
	2 Bytes, 2** Cycles

	     N V A B  D I Z C
	164  1 0 1 0  0 1 0 0
	38   0 0 1 0  0 1 0 0
*/
uint8_t	BPL_REL(_6502 *mos6502) {
	mos6502->PC += 2;
	//printf("BPL_REL");
	if (mos6502->get_flag(mos6502, 'N') == 0) {
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	return 2;
}

/*
	ORA - op0x11
	INDIRECT INDEXING
	2 Bytes, 5* Cycles
*/
uint8_t	ORA_INDY(_6502* mos6502) {
	//printf("ORA_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	mos6502->A |= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return cycles;
}

/*
	ORA - op0x15
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	ORA_ZPX(_6502* mos6502) {
	//printf("ORA_ZPX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) + mos6502->X);
	mos6502->A |= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	ASL - op0x16
	ZERO PAGE X
	2 Bytes, 6 Cycles
*/
uint8_t	ASL_ZPX(_6502* mos6502) {
	//printf("ASL_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint16_t	addr = ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	mos6502->set_flag(mos6502, 'C', (operand >> 0x7) & 0x1);
	operand <<= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, addr, operand);
	mos6502->PC += 2;
	return 6;
}

/*
	CLC - op0x18
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	CLC_IMP(_6502 *mos6502) {
	//printf("CLC_IMP");
	mos6502->set_flag(mos6502, 'C', 0);
	mos6502->PC += 1;
	return 2;
}

/*
	ORA - op0x19
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	ORA_ABSY(_6502 *mos6502) {
	//printf("ORA_ABSY");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t absy_addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	mos6502->A |= mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	ORA - op0x1D
	ABSOLUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	ORA_ABSX(_6502 *mos6502) {
	//printf("ORA_ABSX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	mos6502->A |= mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	ASL - op0x1E
	ABSOLUTE X
	3 Bytes, 7 Cycles
*/
uint8_t	ASL_ABSX(_6502 *mos6502) {
	//printf("ASL_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	mos6502->set_flag(mos6502, 'C', (operand >> 0x7) & 0x1);
	operand <<= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, absx_addr, operand);
	mos6502->PC += 3;
	return 7;
}

/*
	JSR - op0x20
	ABSOLUTE
	3 Bytes, 6 Cycles
*/
uint8_t	JSR_ABS(_6502* mos6502) {
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	// high_byte first
	uint16_t return_addr = mos6502->PC + 2;
	mos6502->push(mos6502, (return_addr >> 0x8) & 0xFF);
	mos6502->push(mos6502, return_addr & 0xFF);
	mos6502->PC = high_byte << 0x8 | low_byte;
	//printf("JSR_ABS");
	return 6;
}

/*
	AND - op0x21
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	AND_INDX(_6502 *mos6502) {
	//printf("AND_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->A &= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 6;
}

/*
	BIT - op0x24
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	BIT_ZP(_6502 *mos6502) {
	//printf("BIT_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	mos6502->set_flag(mos6502, 'Z', (operand & mos6502->A) == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->set_flag(mos6502, 'V', (operand >> 0x6) & 0x1);
	mos6502->PC += 2;
	return 3;
}

/*
	AND - op0x25
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	AND_ZP(_6502 *mos6502) {
	//printf("AND_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, 0x00 << 0x8 | low_byte);
	mos6502->A &= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	ROL - op0x27
	ZERO PAGE
	2 Bytes, 5 Cycles
*/
uint8_t	ROL_ZP(_6502 *mos6502) {
	//printf("ROL_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 8 | low_byte) & 0xFF),
		old_carry = mos6502->get_flag(mos6502, 'C'),
		new_carry = (operand & 0x80) >> 7;
	operand = (operand << 0x1) | old_carry;
	mos6502->set_flag(mos6502, 'C', new_carry);
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, 0x00 << 0x8 | low_byte, operand);
	mos6502->PC += 2;
	return 5;
}

/*
	PLP - op0x28
	IMPLIED
	1 Byte, 4 Cycles
*/
uint8_t	PLP_IMP(_6502 *mos6502) {
	//printf("PLP_IMP");
	mos6502->SR = mos6502->pull(mos6502);
	mos6502->SR &= ~0x10;
	mos6502->SR |= 0x20;
	mos6502->PC += 1;
	return 4;
}

/*
	AND - op0x29
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	AND_IMM(_6502 *mos6502) {
	//printf("AND_IMM");
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->A &= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	ROL - op0x2A
	ACCUMULATOR
	1 Byte, 2 Cycles
*/
uint8_t	ROL_ACC(_6502 *mos6502) {
	//printf("ROL_ACC");
	uint8_t	old_carry = mos6502->get_flag(mos6502, 'C'),
		new_carry = (mos6502->A & 0x80) >> 7;

	mos6502->A = (mos6502->A << 0x1) | old_carry;
	mos6502->set_flag(mos6502, 'C', new_carry);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);


	mos6502->PC += 1;
	return 2;
}

/*
	BIT - op0x2C
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	BIT_ABS(_6502 *mos6502) {
	//printf("BIT_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	       	high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->set_flag(mos6502, 'Z', (operand & mos6502->A) == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->set_flag(mos6502, 'V', (operand >> 0x6) & 0x1);
	mos6502->PC += 3;
	return 4;
}

/*
	AND - op0x2D
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	AND_ABS(_6502 *mos6502) {
	//printf("AND_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->A &= mos6502->bus->cpu_read(mos6502->bus, high_byte << 8 | low_byte);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	ROL - op0x2E
	ABSOLUTE
	3 Bytes, 6 Cycles
*/
uint8_t	ROL_ABS(_6502 *mos6502) {
	//printf("ROL_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 8 | low_byte),
		old_carry = mos6502->get_flag(mos6502, 'C'),
		new_carry = (operand & 0x80) >> 7;

	operand = (operand << 0x1) | old_carry;
	mos6502->set_flag(mos6502, 'C', new_carry);
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);

	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, operand);
	mos6502->PC += 3;
	return 6;
}

/*
	BMI - op0x30
	RELATIVE
	2 Bytes, 2** Cycles
*/
uint8_t	BMI_REL(_6502 *mos6502) {
	//printf("BMI_REL\b");
	mos6502->PC += 2;
	if (mos6502->get_flag(mos6502, 'N')) {
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	return 2;
}

/*
	AND - op0x31
	INDIRECT INDEXED
	2 Bytes, 5* Cycles
*/
uint8_t	AND_INDY(_6502 *mos6502) {
	//printf("AND_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	mos6502->A &= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return cycles;
}

/*
	AND - op0x35
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	AND_ZPX(_6502 *mos6502) {
	//printf("AND_ZPX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF);
	mos6502->A &= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	ROL - op0x36
	ZERO PAGE X
	2 Bytes, 6 Cycles
*/
uint8_t	ROL_ZPX(_6502 *mos6502) {
	//printf("ROL_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint16_t	addr = ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr),
		old_carry = mos6502->get_flag(mos6502, 'C'),
		new_carry = (operand & 0x80) >> 7;

	operand = (operand << 0x1) | old_carry;
	mos6502->set_flag(mos6502, 'C', new_carry);
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);

	mos6502->bus->cpu_write(mos6502->bus, addr, operand);
	mos6502->PC += 2;
	return 6;
}

/*
	SEC - op0x38
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	SEC_IMP(_6502 *mos6502) {
	//printf("SEC_IMP");
	mos6502->set_flag(mos6502, 'C', 1);
	mos6502->PC += 1;
	return 2;
}

/*
	AND - op0x39
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	AND_ABSY(_6502 *mos6502) {
	//printf("AND_ABSY");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t absy_addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	mos6502->A &= mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	AND - op0x3D
	ABSOLUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	AND_ABSX(_6502 *mos6502) {
	//printf("AND_ABSX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	mos6502->A &= mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	ROL - op0x3E
	ABSOLUTE X
	3 Bytes, 7 Cycles
*/
uint8_t	ROL_ABSX(_6502 *mos6502) {
	//printf("ROL_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr),
		old_carry = mos6502->get_flag(mos6502, 'C'),
		new_carry = (operand & 0x80) >> 7;

	operand = (operand << 0x1) | old_carry;
	mos6502->set_flag(mos6502, 'C', new_carry);
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);

	mos6502->bus->cpu_write(mos6502->bus, absx_addr, operand);
	mos6502->PC += 3;
	return 7;
}

/*
	RTI - op0x40
	IMPLIED
	1 Byte, 6 Cycles
*/
uint8_t	RTI_IMP(_6502 *mos6502) {
	//printf("RTI_IMP");
	mos6502->SR = mos6502->pull(mos6502);
	mos6502->SR &= ~0x10;
	mos6502->SR |= 0x20;
	uint8_t	low_byte = mos6502->pull(mos6502);
	mos6502->PC = mos6502->pull(mos6502) << 8 | low_byte;
	return 6;
}

/*
	EOR - op0x41
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	EOR_INDX(_6502 *mos6502) {
	//printf("EOR_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->A ^= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 6;
}

/*
	EOR - op0x45
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	EOR_ZP(_6502 *mos6502) {
	//printf("EOR_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 8 | low_byte) & 0xFF);
	mos6502->A ^= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	LSR - op0x46
	ZERO PAGE
	2 Bytes, 5 Cycles

	    N V A B  D I Z C
	A5  1 0 1 0  0 1 0 1

	A4  1 0 1 0  0 1 0 1 <-
	24  0 0 1 0  0 1 0 0 ->	
*/
uint8_t	LSR_ZP(_6502 *mos6502) {
	//printf("LSR_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 8 | low_byte) & 0xFF);
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, 0x00 << 0x8 | low_byte, operand);
	mos6502->PC += 2;
	return 5;
}

/*
	PHA - op0x48
	IMPLIED
	1 Byte, 3 Cycles
*/
uint8_t	PHA_IMP(_6502 *mos6502) {
	//printf("PHA_IMP");
	mos6502->push(mos6502, mos6502->A);
	mos6502->PC += 1;
	return 3;
}

/*
	EOR - op0x49
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	EOR_IMM(_6502 *mos6502) {
	//printf("EOR_IMM");
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->A ^= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	LSR - op0x4A
	ACCUMULATOR
	1 Byte, 2 Cycles

	    N V A B  D I Z C
	A5  1 0 1 0  0 1 0 1

	A4  1 0 1 0  0 1 0 0 <-
	24  0 0 1 0  0 1 0 0 ->
*/
uint8_t	LSR_ACC(_6502 *mos6502) {
	//printf("LSR_ACC");
	mos6502->set_flag(mos6502, 'C', mos6502->A & 0x1);
	mos6502->A >>= 0x1;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	JMP - op0x4C
	ABSOLUTE
	3 Bytes, 3 Cycles
*/
uint8_t	JMP_ABS(_6502 *mos6502) {
	//printf("JMP_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->PC = high_byte << 0x8 | low_byte;
	return 3;
}

/*
	EOR - op0x4D
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	EOR_ABS(_6502 *mos6502) {
	//printf("EOR_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->A ^= mos6502->bus->cpu_read(mos6502->bus, high_byte << 8 | low_byte);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	LSR - op0x4E
	ABSOLUTE
	3 Bytes, 6 Cycles
*/
uint8_t	LSR_ABS(_6502 *mos6502) {
	//printf("LSR_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 8 | low_byte);
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, operand);
	mos6502->PC += 3;
	return 6;
}

/*
	BVC - op0x50
	RELATIVE
	2 Bytes, 2** Cycles
*/
uint8_t	BVC_REL(_6502 *mos6502) {
	//printf("BVC_REL");
	mos6502->PC += 2;
	if (mos6502->get_flag(mos6502, 'V') == 0) {
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	return 2;
}

/*
	EOR - op0x51
	INDIRECT INDEXED
	2 Bytes, 5* Cycles
*/
uint8_t	EOR_INDY(_6502 *mos6502) {
	//printf("EOR_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	mos6502->A ^= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return cycles;
}

/*
	EOR - op0x55
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	EOR_ZPX(_6502 *mos6502) {
	//printf("EOR_ZPX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, ((0x00 << 8 | low_byte) + mos6502->X) & 0xFF);
	mos6502->A ^= operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	LSR - op0x56
	ZERO PAGE X
	2 Bytes, 6 Cycles
*/
uint8_t	LSR_ZPX(_6502 *mos6502) {
	//printf("LSR_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint16_t	addr = ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, addr, operand);
	mos6502->PC += 2;
	return 6;
}

/*
	CLI - op0x58
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	CLI_IMP(_6502 *mos6502) {
	//printf("CLI_IMP");
	mos6502->set_flag(mos6502, 'I', 0);
	mos6502->PC += 1;
	return 2;
}

/*
	EOR - op0x59
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	EOR_ABSY(_6502 *mos6502) {
	//printf("EOR_ABSY");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t absy_addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	mos6502->A ^= mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	EOR - op0x5D
	ABSOUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	EOR_ABSX(_6502 *mos6502) {
	//printf("EOR_ABSX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	mos6502->A ^= mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	LSR - op0x5E
	ABSOLUTE X
	3 Bytes, 7 Cycles
*/
uint8_t	LSR_ABSX(_6502 *mos6502) {
	//printf("LSR_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, absx_addr, operand);
	mos6502->PC += 3;
	return 7;
}

/*
	RTS - op0x60
	IMPLIED
	1 Byte, 6 Cycles
*/
uint8_t	RTS_IMP(_6502 *mos6502) {
	// low byte first
	uint8_t low_byte = mos6502->pull(mos6502);
	mos6502->PC = mos6502->pull(mos6502) << 8 | low_byte;
	mos6502->PC += 1;
	//printf("RTS_IMP");
	return 6;
}

/*
	ADC - op0x61
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	ADC_INDX(_6502 *mos6502) {
	//printf("ADC_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint16_t	res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	// signOverflow: sameSign (+*..) sameSign = diffSign
	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 6;
}

/*
	ADC - op0x65
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	ADC_ZP(_6502 *mos6502) {
	//printf("ADC_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	uint16_t res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	// signOverflow: sameSign (+*..) sameSign = diffSign
	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));

	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	ROR - op0x66
	ZERO PAGE
	2 Bytes, 5 Cycles
*/
uint8_t	ROR_ZP(_6502 *mos6502) {
	//printf("ROR_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	uint8_t	carry_in = mos6502->get_flag(mos6502, 'C');
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	if (carry_in)	operand |= 0x80;
	else		operand &= ~0x80;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, 0x00 << 0x8 | low_byte, operand);
	mos6502->PC += 2;
	return 5;
}

/*
	PLA - op0x68
	IMPLIED
	1 Byte, 4 Cycles
*/
uint8_t	PLA_IMP(_6502 *mos6502) {
	//printf("PLA_IMP");
	mos6502->A = mos6502->pull(mos6502);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 1;
	return 4;
}

/*
	ADC - op0x69
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	ADC_IMM(_6502 *mos6502) {
	//printf("ADC_IMM");
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint16_t res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	// signOverflow: sameSign (+*..) sameSign = diffSign
	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));

	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	ROR - op0x6A
	ACCUMULATOR
	1 Byte, 2 Cycles

	    N V A B  D I Z C
	01  0 0 0 0  0 0 0 1
	01  0 0 0 0  0 0 0 1 <- 
	80  1 0 0 0  0 0 0 0 ->
*/
uint8_t	ROR_ACC(_6502* mos6502) {
	//printf("ROR_ACC");
	uint8_t	carry_in = mos6502->get_flag(mos6502, 'C');

	mos6502->set_flag(mos6502, 'C', mos6502->A & 0x1);
	mos6502->A >>= 0x1;
	if (carry_in)	mos6502->A |= 0x80;
	else		mos6502->A &= ~0x80;

	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', (mos6502->A >> 0x7) & 0x1);
	mos6502->PC += 1;
	return 2;
}

/*
	JMP - op0x6C
	INDIRECT
	3 Bytes, 5 Cycles
*/
uint8_t	JMP_IND(_6502 *mos6502) {
	//printf("JMP_IND");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint16_t	addr = high_byte << 0x8 | low_byte, high_addr;
	if ((addr & 0xFF) == 0xFF)
		high_addr = addr & 0xFF00;
	else	high_addr = addr + 1;
	mos6502->PC =  mos6502->bus->cpu_read(mos6502->bus, high_addr) << 0x8 |
		mos6502->bus->cpu_read(mos6502->bus, addr);
	return 5;
}

/*
	ADC - op0x6D
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	ADC_ABS(_6502 *mos6502) {
	//printf("ADC_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint16_t res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	// signOverflow: sameSign (+*..) sameSign = diffSign
	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));

	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	ROR - op0x6E
	ABSOLUTE
	3 Bytes, 6 Cycles
*/
uint8_t	ROR_ABS(_6502 *mos6502) {
	//printf("ROR_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint8_t	carry_in = mos6502->get_flag(mos6502, 'C');
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	if (carry_in)	operand |= 0x80;
	else		operand &= ~0x80;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, operand);
	mos6502->PC += 3;
	return 6;
}

/*
	BVS - op0x70
	RELATIVE
	2 Bytes, 2** Cycles
*/
uint8_t	BVS_REL(_6502 *mos6502) {
	//printf("BVS_REL");
	mos6502->PC += 2;
	if (mos6502->get_flag(mos6502, 'V')) {
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	return 2;
}

/*
	ADC - op0x71
	INDIRECT INDEXED
	2 Bytes, 5* Cycles
*/
uint8_t	ADC_INDY(_6502 *mos6502) {
	//printf("ADC_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	uint16_t	res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	// signOverflow: sameSign (+*..) sameSign = diffSign
	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));

	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return cycles;
}

/*
	ADC - op0x75
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	ADC_ZPX(_6502 *mos6502) {
	//printf("ADC_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (low_byte + mos6502->X) & 0xFF);
	uint16_t res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	
	//printf(" opr %04X(%02X) + A %02X + C(%u),", (low_byte + mos6502->X) & 0xFF, operand, mos6502->A, mos6502->get_flag(mos6502, 'C'));

	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);

	mos6502->PC += 2;
	return 4;
}

/*
	ROR - op0x76
	ZERO PAGE X
	2 Bytes, 6 Cycles
*/
uint8_t	ROR_ZPX(_6502 *mos6502) {
	//printf("ROR_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint16_t	addr = ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	uint8_t	carry_in = mos6502->get_flag(mos6502, 'C');
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	if (carry_in)	operand |= 0x80;
	else		operand &= ~0x80;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, addr, operand);
	mos6502->PC += 2;
	return 6;
}

/*
	SEI - op0x78
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	SEI_IMP(_6502 *mos6502) {
	//printf("SEI_IMP");
	mos6502->set_flag(mos6502, 'I', 1);
	mos6502->PC += 1;
	return 2;
}

/*
	ADC - op0x79
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	ADC_ABSY(_6502* mos6502) {
	//printf("ADC_ABSY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2),
		cycles = 4, operand;
	uint16_t absy_addr = high_byte << 0x8 | low_byte, res;
	absy_addr += mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00)); // page crossing
	operand = mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	// signOverflow: sameSign (+*..) sameSign = diffSign
	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));

	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	ADC - op0x7D
	ABSOLUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	ADC_ABSX(_6502 *mos6502) {
	//printf("ADC_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2),
		cycles = 4, operand;
	uint16_t absx_addr = high_byte << 0x8 | low_byte, res;
	absx_addr += mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	res = mos6502->A + operand + mos6502->get_flag(mos6502, 'C');
	// signOverflow: sameSign (+*..) sameSign = diffSign
	mos6502->set_flag(mos6502, 'V', (((operand ^ (res & 0xFF)) & 0x80) && !((operand ^ mos6502->A) & 0x80)));

	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', res > 255);
		mos6502->A = res & 0xFF;
	}
	else {
		uint8_t low = (mos6502->A & 0xF) + (operand & 0xF) + mos6502->get_flag(mos6502, 'C');
		uint8_t high = (mos6502->A >> 4) + (operand >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
			mos6502->set_flag(mos6502, 'C', 1);
		}
		else	mos6502->set_flag(mos6502, 'C', 0);
		mos6502->A = ((high & 0xF) << 4) | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	ROR - op0x7E
	ABSOLUTE X
	3 Bytes, 7 Cycles
*/
uint8_t	ROR_ABSX(_6502 *mos6502) {
	//printf("ROR_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	uint8_t	carry_in = mos6502->get_flag(mos6502, 'C');
	mos6502->set_flag(mos6502, 'C', operand & 0x1);
	operand >>= 0x1;
	if (carry_in)	operand |= 0x80;
	else		operand &= ~0x80;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', (operand >> 0x7) & 0x1);
	mos6502->bus->cpu_write(mos6502->bus, absx_addr, operand);
	mos6502->PC += 3;
	return 7;
}

/*
	STA - op0x81
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	STA_INDX(_6502 *mos6502) {
	//printf("STA_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	/*uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->bus->cpu_write(mos6502->bus, operand, mos6502->A);*/
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, mos6502->A);
	mos6502->PC += 2;
	return 6;
}

/*
	STY - op0x84
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	STY_ZP(_6502 *mos6502) {
	//printf("STY_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->bus->cpu_write(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF, mos6502->Y);
	mos6502->PC += 2;
	return 3;
}

/*
	STA - op0x85
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	STA_ZP(_6502 *mos6502) {
	//printf("STA_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->bus->cpu_write(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF, mos6502->A);
	mos6502->PC += 2;
	return 3;
}

/*
	STX - op0x86
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	STX_ZP(_6502 *mos6502) {
	//printf("STX_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->bus->cpu_write(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF, mos6502->X);
	mos6502->PC += 2;
	return 3;
}

/*
	DEY - op0x88
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	DEY_IMP(_6502* mos6502) {
	//printf("DEY_IMP");
	mos6502->Y -= 1;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	TXA - op0x8A
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	TXA_IMP(_6502 *mos6502) {
	//printf("TXA_IMP");
	mos6502->A = mos6502->X;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	STY - op0x8C
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	STY_ABS(_6502 *mos6502) {
	//printf("STY_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, mos6502->Y);
	mos6502->PC += 3;
	return 4;
}

/*
	STA - op0x8D
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	STA_ABS(_6502 *mos6502) {
	//printf("STA_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, mos6502->A);
	mos6502->PC += 3;
	return 4;
}

/*
	STX - op0x8E
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	STX_ABS(_6502 *mos6502) {
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, mos6502->X);
	//printf("STX_ABS");
	mos6502->PC += 3;
	return 4;
}

/*
	BCC - op0x90
	RELATIVE
	2 Bytes, 2** Cycles
*/
uint8_t	BCC_REL(_6502 *mos6502) {
	//printf("BCC_REL");
	mos6502->PC += 2;
	if (mos6502->get_flag(mos6502, 'C') == 0) {
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	return 2;
}

/*
	STA - op0x91
	INDIRECT INDEXED
	2 Bytes, 6 Cycles
*/
uint8_t	STA_INDY(_6502 *mos6502) {
	//printf("STA_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	mos6502->bus->cpu_write(mos6502->bus, addr, mos6502->A);
	mos6502->PC += 2;
	return 6;
}

/*
	STY - op0x94
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	STY_ZPX(_6502 *mos6502) {
	//printf("STY_ZPX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->bus->cpu_write(mos6502->bus, ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF, mos6502->Y);
	mos6502->PC += 2;
	return 4;
}

/*
	STA - op0x95
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	STA_ZPX(_6502 *mos6502) {
	//printf("STA_ZPX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->bus->cpu_write(mos6502->bus, ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF, mos6502->A);
	mos6502->PC += 2;
	return 4;
}

/*
	STX - op0x96
	ZERO PAGE Y
	2 Bytes, 4 Cycles
*/
uint8_t	STX_ZPY(_6502 *mos6502) {
	//printf("STX_ZPY");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->bus->cpu_write(mos6502->bus, ((0x00 << 0x8 | low_byte) + mos6502->Y) & 0xFF, mos6502->X);
	mos6502->PC += 2;
	return 4;
}

/*
	TYA - op0x98
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	TYA_IMP(_6502 *mos6502) {
	//printf("TYA_IMP");
	mos6502->A = mos6502->Y;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	STA - op0x99
	ABSOLUTE Y
	3 Bytes, 5 Cycles
*/
uint8_t	STA_ABSY(_6502 *mos6502) {
	//printf("STA_ABSY");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->bus->cpu_write(mos6502->bus, (high_byte << 0x8 | low_byte) + mos6502->Y, mos6502->A);
	mos6502->PC += 3;
	return 5;
}

/*
	TXS - op0x9A
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	TXS_IMP(_6502 *mos6502) {
	//printf("TXS_IMP");
	mos6502->SP = mos6502->X;
	mos6502->PC += 1;
	return 2;
}

/*
	STA - op0x9D
	ABSOLUTE X
	3 Bytes, 5 Cycles
*/
uint8_t	STA_ABSX(_6502 *mos6502) {
	//printf("STA_ABSX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	mos6502->bus->cpu_write(mos6502->bus, (high_byte << 0x8 | low_byte) + mos6502->X, mos6502->A);
	mos6502->PC += 3;
	return 5;
}

/*
	LDY - op0xA0
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	LDY_IMM(_6502 *mos6502) {
	//printf("LDY_IMM");
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->Y = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	LDA - op0xA1
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	LDA_INDX(_6502 *mos6502) {
	//printf("LDA_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->A = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC  += 2;
	return 6;
}

/*
	LDX - op0xA2
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	LDX_IMM(_6502 *mos6502) {
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->X = operand;
	//printf("LDX_IMM");
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	LDY - op0xA4
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	LDY_ZP(_6502 *mos6502) {
	//printf("LDY_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	mos6502->Y = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	LDA - op0xA5
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	LDA_ZP(_6502 *mos6502) {
	//printf("LDA_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, low_byte & 0xFF);
	mos6502->A = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	//printf(" $%04X = %02X", low_byte & 0xFF, operand);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	LDX - op0xA6
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	LDX_ZP(_6502 *mos6502) {
	//printf("LDX_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	mos6502->X = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	TAY - op0xA8
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	TAY_IMP(_6502 *mos6502) {
	//printf("TAY_IMP");
	mos6502->Y = mos6502->A;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	LDA - op0xA9
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	LDA_IMM(_6502 *mos6502) {
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->A = operand;
	//printf("LDA_IMM");
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	TAX - op0xAA
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	TAX_IMP(_6502 *mos6502) {
	//printf("TAX_IMP");
	mos6502->X = mos6502->A;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	LDY - op0xAC
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	LDY_ABS(_6502 *mos6502) {
	//printf("LDY_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2),
		operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->Y = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	LDA - op0xAD
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	LDA_ABS(_6502 *mos6502) {
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2),
		operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->A = operand;
	//printf("LDA_ABS");
	//printf(" $%04X = %02X(%u)", high_byte << 0x8 | low_byte, operand, operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	LDX - op0xAE
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	LDX_ABS(_6502 *mos6502) {
	//printf("LDX_ABS");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2),
		operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	mos6502->X = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	BCS - op0xB0
	RELATIVE
	2 Bytes, 2** Cycles
*/
uint8_t	BCS_REL(_6502 *mos6502) {
	//printf("BCS_REL");
	mos6502->PC += 2;
	if (mos6502->get_flag(mos6502, 'C')) {
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	return 2;
}

/*
	LDA - op0xB1
	INDIRECT INDEXED
	2 Bytes, 5* Cycles
*/
uint8_t	LDA_INDY(_6502 *mos6502) {
	//printf("LDA_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	mos6502->A = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return cycles;
}

/*
	LDY - op0xB4
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	LDY_ZPX(_6502 *mos6502) {
	//printf("LDY_ZPX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (low_byte + mos6502->X) & 0xFF);
	mos6502->Y = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	LDA - op0xB5
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	LDA_ZPX(_6502 *mos6502) {
	//printf("LDA_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF);
	mos6502->A = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	LDX - op0xB6
	ZERO PAGE Y
	2 Bytes, 4 Cycles
*/
uint8_t	LDX_ZPY(_6502 *mos6502) {
	//printf("LDX_ZPY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, ((0x00 << 0x8 | low_byte) + mos6502->Y) & 0xFF);
	mos6502->X = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	CLV - op0xB8
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	CLV_IMP(_6502 *mos6502) {
	//printf("CLV_IMP");
	mos6502->set_flag(mos6502, 'V', 0);
	mos6502->PC += 1;
	return 2;
}

/*
	LDA - op0xB9 
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	LDA_ABSY(_6502 *mos6502) {
	//printf("LDA_ABSY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t	absy_addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	mos6502->A = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	TSX - op0xBA
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	TSX_IMP(_6502 *mos6502) {
	//printf("TSX_IMP");
	mos6502->X = mos6502->SP;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	LDY - op0xBC
	ABSOLUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	LDY_ABSX(_6502 *mos6502) {
	//printf("LDY_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	mos6502->Y = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	LDA - op0xBD
	ABSOLUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	LDA_ABSX(_6502 *mos6502) {
	//printf("LDA_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	mos6502->A = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	LDX - op0xBE
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	LDX_ABSY(_6502 *mos6502) {
	//printf("LDX_ABSY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t	absy_addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xF00));
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	mos6502->X = operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	CPY - op0xC0
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	CPY_IMM(_6502 *mos6502) {
	//printf("CPY_IMM");
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint8_t res = mos6502->Y - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->Y >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	CMP - op0xC1
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	CMP_INDX(_6502 *mos6502) {
	//printf("CMP_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint8_t	res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 6;
}

/*
	CPY - op0xC4
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	CPY_ZP(_6502 *mos6502) {
	//printf("CPY_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	uint8_t res = mos6502->Y - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->Y >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	CMP - op0xC5
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	CMP_ZP(_6502 *mos6502) {
	//printf("CMP_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	uint8_t res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	//printf(" %02X - %02X Z(%u)", mos6502->A, operand, mos6502->get_flag(mos6502, 'Z'));
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	DEC - op0xC6
	ZERO PAGE
	2 Bytes, 5 Cycles
*/
uint8_t	DEC_ZP(_6502 *mos6502) {
	//printf("DEC_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	operand -= 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, 0x00 << 0x8 | low_byte, operand);
	mos6502->PC += 2;
	return 5;
}

/*
	INY - op0xC8
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	INY_IMP(_6502 *mos6502) {
	//printf("INY_IMP");
	mos6502->Y += 1;
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->Y & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	CMP - op0xC9
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	CMP_IMM(_6502 *mos6502) {
	//printf("CMP_IMM");
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	uint8_t res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	//printf(" %02X - %02X Z(%u)", mos6502->A, operand, mos6502->get_flag(mos6502, 'Z'));
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	DEX - op0xCA
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	DEX_IMP(_6502 *mos6502) {
	//printf("DEX_IMP");
	mos6502->X -= 1;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	CPY - op0xCC
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	CPY_ABS(_6502 *mos6502) {
	//printf("CPY_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 8 | low_byte);
	uint8_t res = mos6502->Y - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->Y >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->Y == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	CMP - op0xCD
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	CMP_ABS(_6502 *mos6502) {
	//printf("CMP_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint8_t res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	DEC - op0xCE
	ABSOLUTE
	3 Bytes, 6 Cycles
*/
uint8_t	DEC_ABS(_6502 *mos6502) {
	//printf("DEC_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	operand -= 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, operand);
	mos6502->PC += 3;
	return 6;
}

/*
	BNE - op0xD0
	RELATIVE
	2 Bytes, 2** Cycles
*/
uint8_t	BNE_REL(_6502 *mos6502) {
	//printf("BNE_REL Z(%u)", mos6502->get_flag(mos6502, 'Z'));
	mos6502->PC += 2;
	if (mos6502->get_flag(mos6502, 'Z') == 0) {
		//printf(" branched");
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	//printf(" not branched");
	return 2;
}

/*
	CMP - op0xD1
	INDIRECT INDEXED
	2 Bytes, 5* Cycles
*/
uint8_t	CMP_INDY(_6502 *mos6502) {
	//printf("CMP_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	uint8_t res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return cycles;
}

/*
	CMP - op0xD5
	ZERO PAGE X
	2 Bytes, 4 Cycles	
*/
uint8_t	CMP_ZPX(_6502 *mos6502) {
	//printf("CMP_ZPX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF);
	uint8_t res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	DEC - op0xD6
	ZERO PAGE X
	2 Bytes, 6 Cycles
*/
uint8_t	DEC_ZPX(_6502 *mos6502) {
	//printf("DEC_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint16_t	addr = ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	operand -= 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, addr, operand);
	mos6502->PC += 2;
	return 6;
}

/*
	CLD - op0xD8
	IMPLIED
	1 Bytes, 2 Cycles
*/
uint8_t	CLD_IMP(_6502 *mos6502) {
	//printf("CLD_IMP");
	mos6502->set_flag(mos6502, 'D', 0);
	mos6502->PC += 1;
	return 2;
}

/*
	CMP - op0xD9
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	CMP_ABSY(_6502 *mos6502) {
	//printf("CMP_ABSY");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t	absy_addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	uint8_t res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	CMP - op0xDD
	ABSOLUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	CMP_ABSX(_6502 *mos6502) {
	//printf("CMP_ABSX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2), cycles = 4;
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	uint8_t res = mos6502->A - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->A >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->A == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	DEC - op0xDE
	ABSOLUTE X
	3 Bytes, 7 Cycles
*/
uint8_t	DEC_ABSX(_6502 *mos6502) {
	//printf("DEC_ABSX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	operand -= 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, absx_addr, operand);
	mos6502->PC += 3;
	return 7;
}

/*
	CPX - op0xE0
	IMMEDIATE
	2 Bytes, 2 Cycles
*/
uint8_t	CPX_IMM(_6502 *mos6502) {
	//printf("CPX_IMM");
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint8_t res = mos6502->X - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->X >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->X == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	SBC - op0xE1
	INDEXED INDIRECT
	2 Bytes, 6 Cycles
*/
uint8_t	SBC_INDX(_6502 *mos6502) {
	//printf("SBC_INDX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = (low_byte + mos6502->X) & 0xFF;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 6;
}

/*
	CPX - op0xE4
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	CPX_ZP(_6502 *mos6502) {
	//printf("CPX_ZP");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 0x8 | low_byte) & 0xFF);
	uint8_t res = mos6502->X - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->X >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->X == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	SBC - op0xE5
	ZERO PAGE
	2 Bytes, 3 Cycles
*/
uint8_t	SBC_ZP(_6502 *mos6502) {
	//printf("SBC_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 8 | low_byte) & 0xFF);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 3;
}

/*
	INC - op0xE6
	ZERO PAGE
	2 Bytes, 5 Cycles
*/
uint8_t	INC_ZP(_6502 *mos6502) {
	//printf("INC_ZP");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, (0x00 << 8 | low_byte) & 0xFF);
	operand += 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, 0x00 << 8 | low_byte, operand);
	mos6502->PC += 2;
	return 5;
}

/*
	INX - op0xE8
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	INX_IMP(_6502 *mos6502) {
	//printf("INX_IMP");
	mos6502->X += 1;
	mos6502->set_flag(mos6502, 'Z', mos6502->X == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->X & 0x80);
	mos6502->PC += 1;
	return 2;
}

/*
	SBC - op0xE9
	IMMEDIATE
	2 Bytes, 2 Cycles
*/

uint8_t	SBC_IMM(_6502 *mos6502) {
	//printf("SBC_IMM");
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 2;
}

/*
	NOP - op0xEA
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	NOP_IMP(_6502 *mos6502) {
	//printf("NOP_IMP");
	mos6502->PC += 1;
	return 2;
}

/*
	CPX - op0xEC
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	CPX_ABS(_6502 *mos6502) {
	//printf("CPX_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint8_t res = mos6502->X - operand;
	mos6502->set_flag(mos6502, 'C', mos6502->X >= operand);
	mos6502->set_flag(mos6502, 'Z', mos6502->X == operand);
	mos6502->set_flag(mos6502, 'N', res & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	SBC - op0xED
	ABSOLUTE
	3 Bytes, 4 Cycles
*/
uint8_t	SBC_ABS(_6502 *mos6502) {
	//printf("SBC_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return 4;
}

/*
	INC - op0xEE
	ABSOLUTE
	3 Bytes, 6 Cycles
*/
uint8_t	INC_ABS(_6502 *mos6502) {
	//printf("INC_ABS");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, high_byte << 0x8 | low_byte);
	operand += 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, high_byte << 0x8 | low_byte, operand);
	mos6502->PC += 3;
	return 6;
}

/*
	BEQ - op0xF0
	RELATIVE
	2 Bytes, 2** Cycles
*/
uint8_t	BEQ_REL(_6502 *mos6502) {
	//printf("BEQ_REL");
	mos6502->PC += 2;
	if (mos6502->get_flag(mos6502, 'Z')) {
		int8_t operand = (int8_t)mos6502->bus->cpu_read(mos6502->bus, mos6502->PC-1);
		mos6502->PC += operand;
		return 3;
	}
	return 2;
}

/*
	SBC - op0xF1
	INDIRECT INDEXED
	2 Bytes, 5* Cycles
*/
uint8_t	SBC_INDY(_6502 *mos6502) {
	//printf("SBC_INDY");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1), high_byte;
	uint8_t	base = 0x00 << 0x8 | low_byte, cycles = 5;
	low_byte = mos6502->bus->cpu_read(mos6502->bus, base);
	high_byte = mos6502->bus->cpu_read(mos6502->bus, (base+1) & 0xFF);
	uint16_t	addr = (high_byte << 0x8 | low_byte) + mos6502->Y;
	cycles += ((addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return cycles;
}

/*
	SBC - op0xF5
	ZERO PAGE X
	2 Bytes, 4 Cycles
*/
uint8_t	SBC_ZPX(_6502 *mos6502) {
	//printf("SBC_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
		operand = mos6502->bus->cpu_read(mos6502->bus, ((0x00 << 8 | low_byte) + mos6502->X) & 0xFF);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 2;
	return 4;
}

/*
	INC - op0xF6
	ZERO PAGE X
	2 Bytes, 6 Cycles
*/
uint8_t	INC_ZPX(_6502 *mos6502) {
	//printf("INC_ZPX");
	uint8_t	low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1);
	uint16_t	addr = ((0x00 << 0x8 | low_byte) + mos6502->X) & 0xFF;
	uint8_t	operand = mos6502->bus->cpu_read(mos6502->bus, addr);
	operand += 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, addr, operand);
	mos6502->PC += 2;
	return 6;
}

/*
	SED - op0xF8
	IMPLIED
	1 Byte, 2 Cycles
*/
uint8_t	SED_IMP(_6502 *mos6502) {
	//printf("SED_IMP");
	mos6502->set_flag(mos6502, 'D', 1);
	mos6502->PC += 1;
	return 2;
}

/*
	SBC - op0xF9
	ABSOLUTE Y
	3 Bytes, 4* Cycles
*/
uint8_t	SBC_ABSY(_6502 *mos6502) {
	//printf("SBC_ABSY");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2),
	        cycles = 4;
	uint16_t absy_addr = high_byte << 0x8 | low_byte;
	absy_addr += mos6502->Y;
	cycles += ((absy_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, absy_addr);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	SBC - op0xFD
	ABSOLUTE X
	3 Bytes, 4* Cycles
*/
uint8_t	SBC_ABSX(_6502 *mos6502) {
	//printf("SBC_ABSX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2),
	        cycles = 4;
	uint16_t absx_addr = high_byte << 0x8 | low_byte;
	absx_addr += mos6502->X;
	cycles += ((absx_addr & 0xFF00) != ((high_byte << 0x8 | low_byte) & 0xFF00));
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	uint8_t old_a = mos6502->A;
	uint16_t temp = mos6502->A - operand - (1 - mos6502->get_flag(mos6502, 'C'));
	mos6502->set_flag(mos6502, 'V', ((old_a ^ operand) & 0x80) && ((old_a ^ temp) & 0x80));
	if (!mos6502->get_flag(mos6502, 'D')) {
		mos6502->set_flag(mos6502, 'C', temp < 0x100);
		mos6502->A = temp & 0xFF;
	}
	else {
		int8_t low = (mos6502->A & 0xF) - (operand & 0xF) - (1 - mos6502->get_flag(mos6502, 'C'));
		int8_t high = (mos6502->A >> 4) - (operand >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
			mos6502->set_flag(mos6502, 'C', 0);
		}
		else	mos6502->set_flag(mos6502, 'C', 1);
		mos6502->A = (high & 0xF) << 4 | (low & 0xF);
	}
	mos6502->set_flag(mos6502, 'Z', mos6502->A == 0);
	mos6502->set_flag(mos6502, 'N', mos6502->A & 0x80);
	mos6502->PC += 3;
	return cycles;
}

/*
	INC - op0xFE
	ABSOLUTE X
	3 Bytes, 7 Cycles
*/
uint8_t	INC_ABSX(_6502 *mos6502) {
	//printf("INC_ABSX");
	uint8_t low_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+1),
	        high_byte = mos6502->bus->cpu_read(mos6502->bus, mos6502->PC+2);
	uint16_t	absx_addr = (high_byte << 0x8 | low_byte) + mos6502->X;
	uint8_t operand = mos6502->bus->cpu_read(mos6502->bus, absx_addr);
	operand += 1;
	mos6502->set_flag(mos6502, 'Z', operand == 0);
	mos6502->set_flag(mos6502, 'N', operand & 0x80);
	mos6502->bus->cpu_write(mos6502->bus, absx_addr, operand);
	
	mos6502->PC += 3;
	return 7;
}

//// /// /	*ILLEGAL OPCODES

/*  op0x02, 2By, 2Cy */
uint8_t   OP_02(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x03, 2By, 8Cy */
uint8_t   OP_03(_6502 *mos6502) {
	//printf("*SLO");
	mos6502->PC += 2;
	return 8;
}

/*  op0x04, 2By, 3Cy */
uint8_t   OP_04(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 3;
}

/*  op0x07, 2By, 5Cy */
uint8_t   OP_07(_6502 *mos6502) {
	//printf("*SLO");
	mos6502->PC += 2;
	return 5;
}

/*  op0x0B, 2By, 2Cy */
uint8_t   OP_0B(_6502 *mos6502) {
	//printf("*ANC");
	mos6502->PC += 2;
	return 2;
}

/*  op0x0C, 3By, 4Cy */
uint8_t   OP_0C(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 3;
	return 4;
}

/*  op0x0F, 3By, 6Cy */
uint8_t   OP_0F(_6502 *mos6502) {
	//printf("*SLO");
	mos6502->PC += 3;
	return 6;
}

/*  op0x12, 2By, 2Cy */
uint8_t   OP_12(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x13, 2By, 8Cy */
uint8_t   OP_13(_6502 *mos6502) {
	//printf("*SLO");
	mos6502->PC += 2;
	return 8;
}

/*  op0x14, 2By, 4Cy */
uint8_t   OP_14(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 4;
}

/*  op0x17, 2By, 6Cy */
uint8_t   OP_17(_6502 *mos6502) {
	//printf("*SLO");
	mos6502->PC += 2;
	return 6;
}

/*  op0x1A, 1By, 2Cy */
uint8_t   OP_1A(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 1;
	return 2;
}

/*  op0x1B, 3By, 7Cy */
uint8_t   OP_1B(_6502 *mos6502) {
	//printf("*SLO");
	mos6502->PC += 3;
	return 7;
}

/*  op0x1C, 3By, 4Cy */
uint8_t   OP_1C(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 3;
	return 4;
}

/*  op0x1F, 3By, 7Cy */
uint8_t   OP_1F(_6502 *mos6502) {
	//printf("*SLO");
	mos6502->PC += 3;
	return 7;
}

/*  op0x22, 2By, 2Cy */
uint8_t   OP_22(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x23, 2By, 8Cy */
uint8_t   OP_23(_6502 *mos6502) {
	//printf("*RLA");
	mos6502->PC += 2;
	return 8;
}

/*  op0x27, 2By, 5Cy */
uint8_t   OP_27(_6502 *mos6502) {
	//printf("*RLA");
	mos6502->PC += 2;
	return 5;
}

/*  op0x2B, 2By, 2Cy */
uint8_t   OP_2B(_6502 *mos6502) {
	//printf("*ANC");
	mos6502->PC += 2;
	return 2;
}

/*  op0x2F, 3By, 6Cy */
uint8_t   OP_2F(_6502 *mos6502) {
	//printf("*RLA");
	mos6502->PC += 3;
	return 6;
}

/*  op0x32, 2By, 2Cy */
uint8_t   OP_32(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x33, 2By, 8Cy */
uint8_t   OP_33(_6502 *mos6502) {
	//printf("*RLA");
	mos6502->PC += 2;
	return 8;
}

/*  op0x34, 2By, 4Cy */
uint8_t   OP_34(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 4;
}

/*  op0x37, 2By, 6Cy */
uint8_t   OP_37(_6502 *mos6502) {
	//printf("*RLA");
	mos6502->PC += 2;
	return 6;
}

/*  op0x3A, 1By, 2Cy */
uint8_t   OP_3A(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 1;
	return 2;
}

/*  op0x3B, 3By, 7Cy */
uint8_t   OP_3B(_6502 *mos6502) {
	//printf("*RLA");
	mos6502->PC += 3;
	return 7;
}

/*  op0x3C, 3By, 4Cy */
uint8_t   OP_3C(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 3;
	return 4;
}

/*  op0x3F, 3By, 7Cy */
uint8_t   OP_3F(_6502 *mos6502) {
	//printf("*RLA");
	mos6502->PC += 3;
	return 7;
}

/*  op0x42, 2By, 2Cy */
uint8_t   OP_42(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x43, 2By, 8Cy */
uint8_t   OP_43(_6502 *mos6502) {
	//printf("*SRE");
	mos6502->PC += 2;
	return 8;
}

/*  op0x44, 2By, 3Cy */
uint8_t   OP_44(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 3;
}

/*  op0x47, 2By, 5Cy */
uint8_t   OP_47(_6502 *mos6502) {
	//printf("*SRE");
	mos6502->PC += 2;
	return 5;
}

/*  op0x4B, 2By, 2Cy */
uint8_t   OP_4B(_6502 *mos6502) {
	//printf("*ALR");
	mos6502->PC += 2;
	return 2;
}

/*  op0x4F, 3By, 6Cy */
uint8_t   OP_4F(_6502 *mos6502) {
	//printf("*SRE");
	mos6502->PC += 3;
	return 6;
}

/*  op0x52, 2By, 2Cy */
uint8_t   OP_52(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x53, 2By, 8Cy */
uint8_t   OP_53(_6502 *mos6502) {
	//printf("*SRE");
	mos6502->PC += 2;
	return 8;
}

/*  op0x54, 2By, 4Cy */
uint8_t   OP_54(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 4;
}

/*  op0x57, 2By, 6Cy */
uint8_t   OP_57(_6502 *mos6502) {
	//printf("*SRE");
	mos6502->PC += 2;
	return 6;
}

/*  op0x5A, 1By, 2Cy */
uint8_t   OP_5A(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 1;
	return 2;
}

/*  op0x5B, 3By, 7Cy */
uint8_t   OP_5B(_6502 *mos6502) {
	//printf("*SRE");
	mos6502->PC += 3;
	return 7;
}

/*  op0x5C, 3By, 4Cy */
uint8_t   OP_5C(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 3;
	return 4;
}

/*  op0x5F, 3By, 7Cy */
uint8_t   OP_5F(_6502 *mos6502) {
	//printf("*SRE");
	mos6502->PC += 3;
	return 7;
}

/*  op0x62, 2By, 2Cy */
uint8_t   OP_62(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x63, 2By, 8Cy */
uint8_t   OP_63(_6502 *mos6502) {
	//printf("*RRA");
	mos6502->PC += 2;
	return 8;
}

/*  op0x64, 2By, 3Cy */
uint8_t   OP_64(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 3;
}

/*  op0x67, 2By, 5Cy */
uint8_t   OP_67(_6502 *mos6502) {
	//printf("*RRA");
	mos6502->PC += 2;
	return 5;
}

/*  op0x6B, 2By, 2Cy */
uint8_t   OP_6B(_6502 *mos6502) {
	//printf("*ARR");
	mos6502->PC += 2;
	return 2;
}

/*  op0x6F, 3By, 6Cy */
uint8_t   OP_6F(_6502 *mos6502) {
	//printf("*RRA");
	mos6502->PC += 3;
	return 6;
}

/*  op0x72, 2By, 2Cy */
uint8_t   OP_72(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x73, 2By, 8Cy */
uint8_t   OP_73(_6502 *mos6502) {
	//printf("*RRA");
	mos6502->PC += 2;
	return 8;
}

/*  op0x74, 2By, 4Cy */
uint8_t   OP_74(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 4;
}

/*  op0x77, 2By, 6Cy */
uint8_t   OP_77(_6502 *mos6502) {
	//printf("*RRA");
	mos6502->PC += 2;
	return 6;
}

/*  op0x7A, 1By, 2Cy */
uint8_t   OP_7A(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 1;
	return 2;
}

/*  op0x7B, 3By, 7Cy */
uint8_t   OP_7B(_6502 *mos6502) {
	//printf("*RRA");
	mos6502->PC += 3;
	return 7;
}

/*  op0x7C, 3By, 4Cy */
uint8_t   OP_7C(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 3;
	return 4;
}

/*  op0x7F, 3By, 7Cy */
uint8_t   OP_7F(_6502 *mos6502) {
	//printf("*RRA");
	mos6502->PC += 3;
	return 7;
}

/*  op0x80, 2By, 2Cy */
uint8_t   OP_80(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 2;
}

/*  op0x82, 2By, 2Cy */
uint8_t   OP_82(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 2;
}

/*  op0x83, 2By, 6Cy */
uint8_t   OP_83(_6502 *mos6502) {
	//printf("*SAX");
	mos6502->PC += 2;
	return 6;
}

/*  op0x87, 2By, 3Cy */
uint8_t   OP_87(_6502 *mos6502) {
	//printf("*SAX");
	mos6502->PC += 2;
	return 3;
}

/*  op0x89, 2By, 2Cy */
uint8_t   OP_89(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 2;
}

/*  op0x8B, 2By, 2Cy */
uint8_t   OP_8B(_6502 *mos6502) {
	//printf("*XAA");
	mos6502->PC += 2;
	return 2;
}

/*  op0x8F, 3By, 4Cy */
uint8_t   OP_8F(_6502 *mos6502) {
	//printf("*SAX");
	mos6502->PC += 3;
	return 4;
}

/*  op0x92, 2By, 2Cy */
uint8_t   OP_92(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0x93, 2By, 6Cy */
uint8_t   OP_93(_6502 *mos6502) {
	//printf("*SHA");
	mos6502->PC += 2;
	return 6;
}

/*  op0x94, 2By, 4Cy */
uint8_t   OP_94(_6502 *mos6502) {
	//printf("*STY");
	mos6502->PC += 2;
	return 4;
}

/*  op0x97, 2By, 4Cy */
uint8_t   OP_97(_6502 *mos6502) {
	//printf("*SAX");
	mos6502->PC += 2;
	return 4;
}

/*  op0x9B, 3By, 5Cy */
uint8_t   OP_9B(_6502 *mos6502) {
	//printf("*TAS");
	mos6502->PC += 3;
	return 5;
}

/*  op0x9C, 3By, 5Cy */
uint8_t   OP_9C(_6502 *mos6502) {
	//printf("*SHY");
	mos6502->PC += 3;
	return 5;
}

/*  op0x9E, 3By, 5Cy */
uint8_t   OP_9E(_6502 *mos6502) {
	//printf("*SHX");
	mos6502->PC += 3;
	return 5;
}

/*  op0x9F, 3By, 5Cy */
uint8_t   OP_9F(_6502 *mos6502) {
	//printf("*SHA");
	mos6502->PC += 3;
	return 5;
}

/*  op0xA3, 2By, 6Cy */
uint8_t   OP_A3(_6502 *mos6502) {
	//printf("*LAX");
	mos6502->PC += 2;
	return 6;
}

/*  op0xA7, 2By, 3Cy */
uint8_t   OP_A7(_6502 *mos6502) {
	//printf("*LAX");
	mos6502->PC += 2;
	return 3;
}

/*  op0xAB, 2By, 2Cy */
uint8_t   OP_AB(_6502 *mos6502) {
	//printf("*LAX");
	mos6502->PC += 2;
	return 2;
}

/*  op0xAF, 3By, 4Cy */
uint8_t   OP_AF(_6502 *mos6502) {
	//printf("*LAX");
	mos6502->PC += 3;
	return 4;
}

/*  op0xB2, 2By, 2Cy */
uint8_t   OP_B2(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0xB3, 2By, 5Cy */
uint8_t   OP_B3(_6502 *mos6502) {
	//printf("*LAX");
	mos6502->PC += 2;
	return 5;
}

/*  op0xB4, 2By, 4Cy */
uint8_t   OP_B4(_6502 *mos6502) {
	//printf("*LDY");
	mos6502->PC += 2;
	return 4;
}

/*  op0xB7, 2By, 4Cy */
uint8_t   OP_B7(_6502 *mos6502) {
	//printf("*LAX");
	mos6502->PC += 2;
	return 4;
}

/*  op0xBB, 3By, 4Cy */
uint8_t   OP_BB(_6502 *mos6502) {
	//printf("*LAS");
	mos6502->PC += 3;
	return 4;
}

/*  op0xBF, 3By, 4Cy */
uint8_t   OP_BF(_6502 *mos6502) {
	//printf("*LAX");
	mos6502->PC += 3;
	return 4;
}

/*  op0xC2, 2By, 2Cy */
uint8_t   OP_C2(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 2;
}

/*  op0xC3, 2By, 8Cy */
uint8_t   OP_C3(_6502 *mos6502) {
	//printf("*DCP");
	mos6502->PC += 2;
	return 8;
}

/*  op0xC7, 2By, 5Cy */
uint8_t   OP_C7(_6502 *mos6502) {
	//printf("*DCP");
	mos6502->PC += 2;
	return 5;
}

/*  op0xCB, 2By, 2Cy */
uint8_t   OP_CB(_6502 *mos6502) {
	//printf("*AXS");
	mos6502->PC += 2;
	return 2;
}

/*  op0xCF, 3By, 6Cy */
uint8_t   OP_CF(_6502 *mos6502) {
	//printf("*DCP");
	mos6502->PC += 3;
	return 6;
}

/*  op0xD2, 2By, 2Cy */
uint8_t   OP_D2(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0xD3, 2By, 8Cy */
uint8_t   OP_D3(_6502 *mos6502) {
	//printf("*DCP");
	mos6502->PC += 2;
	return 8;
}

/*  op0xD4, 2By, 4Cy */
uint8_t   OP_D4(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 4;
}

/*  op0xD7, 2By, 6Cy */
uint8_t   OP_D7(_6502 *mos6502) {
	//printf("*DCP");
	mos6502->PC += 2;
	return 6;
}

/*  op0xDA, 1By, 2Cy */
uint8_t   OP_DA(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 1;
	return 2;
}

/*  op0xDB, 3By, 7Cy */
uint8_t   OP_DB(_6502 *mos6502) {
	//printf("*DCP");
	mos6502->PC += 3;
	return 7;
}

/*  op0xDC, 3By, 4Cy */
uint8_t   OP_DC(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 3;
	return 4;
}

/*  op0xDF, 3By, 7Cy */
uint8_t   OP_DF(_6502 *mos6502) {
	//printf("*DCP");
	mos6502->PC += 3;
	return 7;
}

/*  op0xE2, 2By, 2Cy */
uint8_t   OP_E2(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 2;
}

/*  op0xE3, 2By, 8Cy */
uint8_t   OP_E3(_6502 *mos6502) {
	//printf("*ISC");
	mos6502->PC += 2;
	return 8;
}

/*  op0xE7, 2By, 5Cy */
uint8_t   OP_E7(_6502 *mos6502) {
	//printf("*ISC");
	mos6502->PC += 2;
	return 5;
}

/*  op0xEB, 2By, 2Cy */
uint8_t   OP_EB(_6502 *mos6502) {
	//printf("*SBC");
	mos6502->PC += 2;
	return 2;
}

/*  op0xEF, 3By, 6Cy */
uint8_t   OP_EF(_6502 *mos6502) {
	//printf("*ISC");
	mos6502->PC += 3;
	return 6;
}

/*  op0xF2, 2By, 2Cy */
uint8_t   OP_F2(_6502 *mos6502) {
	//printf("*KIL/JAM");
	mos6502->PC += 2;
	return 2;
}

/*  op0xF3, 2By, 8Cy */
uint8_t   OP_F3(_6502 *mos6502) {
	//printf("*ISC");
	mos6502->PC += 2;
	return 8;
}

/*  op0xF4, 2By, 4Cy */
uint8_t   OP_F4(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 2;
	return 4;
}

/*  op0xF7, 2By, 6Cy */
uint8_t   OP_F7(_6502 *mos6502) {
	//printf("*ISC");
	mos6502->PC += 2;
	return 6;
}

/*  op0xFA, 1By, 2Cy */
uint8_t   OP_FA(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 1;
	return 2;
}

/*  op0xFB, 3By, 7Cy */
uint8_t   OP_FB(_6502 *mos6502) {
	//printf("*ISC");
	mos6502->PC += 3;
	return 7;
}

/*  op0xFC, 3By, 4Cy */
uint8_t   OP_FC(_6502 *mos6502) {
	//printf("*NOP");
	mos6502->PC += 3;
	return 4;
}

/*  op0xFF, 3By, 7Cy */
uint8_t   OP_FF(_6502 *mos6502) {
	//printf("*ISC");
	mos6502->PC += 3;
	return 7;
}

/// / //	LOADING

void	load_instructions(_6502* mos6502){
	//  MSD 0
	mos6502->opcodes[0x00] = BRK_IMP;
	mos6502->opcodes[0x01] = ORA_INDX;
	mos6502->opcodes[0x02] = OP_02;
	mos6502->opcodes[0x03] = OP_03;
	mos6502->opcodes[0x04] = OP_04;
	mos6502->opcodes[0x05] = ORA_ZP;
	mos6502->opcodes[0x06] = ASL_ZP;
	mos6502->opcodes[0x07] = OP_07;
	mos6502->opcodes[0x08] = PHP_IMP;
	mos6502->opcodes[0x09] = ORA_IMM;
	mos6502->opcodes[0x0A] = ASL_ACC;
	mos6502->opcodes[0x0B] = OP_0B;
	mos6502->opcodes[0x0C] = OP_0C;
	mos6502->opcodes[0x0D] = ORA_ABS;
	mos6502->opcodes[0x0E] = ASL_ABS;
	mos6502->opcodes[0x0F] = OP_0F;
	//  MSD 1 
	mos6502->opcodes[0x10] = BPL_REL;
	mos6502->opcodes[0x11] = ORA_INDY;
	mos6502->opcodes[0x12] = OP_12;
	mos6502->opcodes[0x13] = OP_13;
	mos6502->opcodes[0x14] = OP_14;
	mos6502->opcodes[0x15] = ORA_ZPX;
	mos6502->opcodes[0x16] = ASL_ZPX;
	mos6502->opcodes[0x17] = OP_17;
	mos6502->opcodes[0x18] = CLC_IMP;
	mos6502->opcodes[0x19] = ORA_ABSY;
	mos6502->opcodes[0x1A] = OP_1A;
	mos6502->opcodes[0x1B] = OP_1B;
	mos6502->opcodes[0x1C] = OP_1C;
	mos6502->opcodes[0x1D] = ORA_ABSX;
	mos6502->opcodes[0x1E] = ASL_ABSX;
	mos6502->opcodes[0x1F] = OP_1F;
	//  MSD 2
	mos6502->opcodes[0x20] = JSR_ABS;
	mos6502->opcodes[0x21] = AND_INDX;
	mos6502->opcodes[0x22] = OP_22;
	mos6502->opcodes[0x23] = OP_23;
	mos6502->opcodes[0x24] = BIT_ZP;
	mos6502->opcodes[0x25] = AND_ZP;
	mos6502->opcodes[0x26] = ROL_ZP;
	mos6502->opcodes[0x27] = OP_27;
	mos6502->opcodes[0x28] = PLP_IMP;
	mos6502->opcodes[0x29] = AND_IMM;
	mos6502->opcodes[0x2A] = ROL_ACC;
	mos6502->opcodes[0x2B] = OP_2B;
	mos6502->opcodes[0x2C] = BIT_ABS;
	mos6502->opcodes[0x2D] = AND_ABS;
	mos6502->opcodes[0x2E] = ROL_ABS;
	mos6502->opcodes[0x2F] = OP_2F;
	//  MSD 3
	mos6502->opcodes[0x30] = BMI_REL;
	mos6502->opcodes[0x31] = AND_INDY;
	mos6502->opcodes[0x32] = OP_32;
	mos6502->opcodes[0x33] = OP_33;
	mos6502->opcodes[0x34] = OP_34;
	mos6502->opcodes[0x35] = AND_ZPX;
	mos6502->opcodes[0x36] = ROL_ZPX;
	mos6502->opcodes[0x37] = OP_37;
	mos6502->opcodes[0x38] = SEC_IMP;
	mos6502->opcodes[0x39] = AND_ABSY;
	mos6502->opcodes[0x3A] = OP_3A;
	mos6502->opcodes[0x3B] = OP_3B;
	mos6502->opcodes[0x3C] = OP_3C;
	mos6502->opcodes[0x3D] = AND_ABSX;
	mos6502->opcodes[0x3E] = ROL_ABSX;
	mos6502->opcodes[0x3F] = OP_3F;
	//  MSD 4
	mos6502->opcodes[0x40] = RTI_IMP;
	mos6502->opcodes[0x41] = EOR_INDX;
	mos6502->opcodes[0x42] = OP_42;
	mos6502->opcodes[0x43] = OP_43;
	mos6502->opcodes[0x44] = OP_44;
	mos6502->opcodes[0x45] = EOR_ZP;
	mos6502->opcodes[0x46] = LSR_ZP;
	mos6502->opcodes[0x47] = OP_47;
	mos6502->opcodes[0x48] = PHA_IMP;
	mos6502->opcodes[0x49] = EOR_IMM;
	mos6502->opcodes[0x4A] = LSR_ACC;
	mos6502->opcodes[0x4B] = OP_4B;
	mos6502->opcodes[0x4C] = JMP_ABS;
	mos6502->opcodes[0x4D] = EOR_ABS;
	mos6502->opcodes[0x4E] = LSR_ABS;
	mos6502->opcodes[0x4F] = OP_4F;
	//  MSD 5
	mos6502->opcodes[0x50] = BVC_REL;
	mos6502->opcodes[0x51] = EOR_INDY;
	mos6502->opcodes[0x52] = OP_52;
	mos6502->opcodes[0x53] = OP_53;
	mos6502->opcodes[0x54] = OP_54;
	mos6502->opcodes[0x55] = EOR_ZPX;
	mos6502->opcodes[0x56] = LSR_ZPX;
	mos6502->opcodes[0x57] = OP_57;
	mos6502->opcodes[0x58] = CLI_IMP;
	mos6502->opcodes[0x59] = EOR_ABSY;
	mos6502->opcodes[0x5A] = OP_5A;
	mos6502->opcodes[0x5B] = OP_5B;
	mos6502->opcodes[0x5C] = OP_5C;
	mos6502->opcodes[0x5D] = EOR_ABSX;
	mos6502->opcodes[0x5E] = LSR_ABSX;
	mos6502->opcodes[0x5F] = OP_5F;
	//  MSD 6
	mos6502->opcodes[0x60] = RTS_IMP;
	mos6502->opcodes[0x61] = ADC_INDX;
	mos6502->opcodes[0x62] = OP_62;
	mos6502->opcodes[0x63] = OP_63;
	mos6502->opcodes[0x64] = OP_64;
	mos6502->opcodes[0x65] = ADC_ZP;
	mos6502->opcodes[0x66] = ROR_ZP;
	mos6502->opcodes[0x67] = OP_67;
	mos6502->opcodes[0x68] = PLA_IMP;
	mos6502->opcodes[0x69] = ADC_IMM;
	mos6502->opcodes[0x6A] = ROR_ACC;
	mos6502->opcodes[0x6B] = OP_6B;
	mos6502->opcodes[0x6C] = JMP_IND;
	mos6502->opcodes[0x6D] = ADC_ABS;
	mos6502->opcodes[0x6E] = ROR_ABS;
	mos6502->opcodes[0x6F] = OP_6F;
	//  MSD 7
	mos6502->opcodes[0x70] = BVS_REL;
	mos6502->opcodes[0x71] = ADC_INDY;
	mos6502->opcodes[0x72] = OP_72;
	mos6502->opcodes[0x73] = OP_73;
	mos6502->opcodes[0x74] = OP_74;
	mos6502->opcodes[0x75] = ADC_ZPX;
	mos6502->opcodes[0x76] = ROR_ZPX;
	mos6502->opcodes[0x77] = OP_77;
	mos6502->opcodes[0x78] = SEI_IMP;
	mos6502->opcodes[0x79] = ADC_ABSY;
	mos6502->opcodes[0x7A] = OP_7A;
	mos6502->opcodes[0x7B] = OP_7B;
	mos6502->opcodes[0x7C] = OP_7C;
	mos6502->opcodes[0x7D] = ADC_ABSX;
	mos6502->opcodes[0x7E] = ROR_ABSX;
	mos6502->opcodes[0x7F] = OP_7F;
	//  MSD 8
	mos6502->opcodes[0x80] = OP_80;
	mos6502->opcodes[0x81] = STA_INDX;
	mos6502->opcodes[0x82] = OP_82;
	mos6502->opcodes[0x83] = OP_83;
	mos6502->opcodes[0x84] = STY_ZP;
	mos6502->opcodes[0x85] = STA_ZP;
	mos6502->opcodes[0x86] = STX_ZP;
	mos6502->opcodes[0x87] = OP_87;
	mos6502->opcodes[0x88] = DEY_IMP;
	mos6502->opcodes[0x89] = OP_89;
	mos6502->opcodes[0x8A] = TXA_IMP;
	mos6502->opcodes[0x8B] = OP_8B;
	mos6502->opcodes[0x8C] = STY_ABS;
	mos6502->opcodes[0x8D] = STA_ABS;
	mos6502->opcodes[0x8E] = STX_ABS;
	mos6502->opcodes[0x8F] = OP_8F;
	//  MSD 9
	mos6502->opcodes[0x90] = BCC_REL;
	mos6502->opcodes[0x91] = STA_INDY;
	mos6502->opcodes[0x92] = OP_92;
	mos6502->opcodes[0x93] = OP_93;
	mos6502->opcodes[0x94] = STY_ZPX;
	mos6502->opcodes[0x95] = STA_ZPX;
	mos6502->opcodes[0x96] = STX_ZPY;
	mos6502->opcodes[0x97] = OP_97;
	mos6502->opcodes[0x98] = TYA_IMP;
	mos6502->opcodes[0x99] = STA_ABSY;
	mos6502->opcodes[0x9A] = TXS_IMP;
	mos6502->opcodes[0x9B] = OP_9B;
	mos6502->opcodes[0x9C] = OP_9C;
	mos6502->opcodes[0x9D] = STA_ABSX;
	mos6502->opcodes[0x9E] = OP_9E;
	mos6502->opcodes[0x9F] = OP_9F;
	//  MSD A
	mos6502->opcodes[0xA0] = LDY_IMM;
	mos6502->opcodes[0xA1] = LDA_INDX;
	mos6502->opcodes[0xA2] = LDX_IMM;
	mos6502->opcodes[0xA3] = OP_A3;
	mos6502->opcodes[0xA4] = LDY_ZP;
	mos6502->opcodes[0xA5] = LDA_ZP;
	mos6502->opcodes[0xA6] = LDX_ZP;
	mos6502->opcodes[0xA7] = OP_A7;
	mos6502->opcodes[0xA8] = TAY_IMP;
	mos6502->opcodes[0xA9] = LDA_IMM;
	mos6502->opcodes[0xAA] = TAX_IMP;
	mos6502->opcodes[0xAB] = OP_AB;
	mos6502->opcodes[0xAC] = LDY_ABS;
	mos6502->opcodes[0xAD] = LDA_ABS;
	mos6502->opcodes[0xAE] = LDX_ABS;
	mos6502->opcodes[0xAF] = OP_AF;
	//  MSD B
	mos6502->opcodes[0xB0] = BCS_REL;
	mos6502->opcodes[0xB1] = LDA_INDY;
	mos6502->opcodes[0xB2] = OP_B2;
	mos6502->opcodes[0xB3] = OP_B3;
	mos6502->opcodes[0xB4] = LDY_ZPX;
	mos6502->opcodes[0xB5] = LDA_ZPX;
	mos6502->opcodes[0xB6] = LDX_ZPY;
	mos6502->opcodes[0xB7] = OP_B7;
	mos6502->opcodes[0xB8] = CLV_IMP;
	mos6502->opcodes[0xB9] = LDA_ABSY;
	mos6502->opcodes[0xBA] = TSX_IMP;
	mos6502->opcodes[0xBB] = OP_BB;
	mos6502->opcodes[0xBC] = LDY_ABSX;
	mos6502->opcodes[0xBD] = LDA_ABSX;
	mos6502->opcodes[0xBE] = LDX_ABSY;
	mos6502->opcodes[0xBF] = OP_BF;
	//  MSD C
	mos6502->opcodes[0xC0] = CPY_IMM;
	mos6502->opcodes[0xC1] = CMP_INDX;
	mos6502->opcodes[0xC2] = OP_C2;
	mos6502->opcodes[0xC3] = OP_C3;
	mos6502->opcodes[0xC4] = CPY_ZP;
	mos6502->opcodes[0xC5] = CMP_ZP;
	mos6502->opcodes[0xC6] = DEC_ZP;
	mos6502->opcodes[0xC7] = OP_C7;
	mos6502->opcodes[0xC8] = INY_IMP;
	mos6502->opcodes[0xC9] = CMP_IMM;
	mos6502->opcodes[0xCA] = DEX_IMP;
	mos6502->opcodes[0xCB] = OP_CB;
	mos6502->opcodes[0xCC] = CPY_ABS;
	mos6502->opcodes[0xCD] = CMP_ABS;
	mos6502->opcodes[0xCE] = DEC_ABS;
	mos6502->opcodes[0xCF] = OP_CF;
	//  MSD D
	mos6502->opcodes[0xD0] = BNE_REL;
	mos6502->opcodes[0xD1] = CMP_INDY;
	mos6502->opcodes[0xD2] = OP_D2;
	mos6502->opcodes[0xD3] = OP_D3;
	mos6502->opcodes[0xD4] = OP_D4;
	mos6502->opcodes[0xD5] = CMP_ZPX;
	mos6502->opcodes[0xD6] = DEC_ZPX;
	mos6502->opcodes[0xD7] = OP_D7;
	mos6502->opcodes[0xD8] = CLD_IMP;
	mos6502->opcodes[0xD9] = CMP_ABSY;
	mos6502->opcodes[0xDA] = OP_DA;
	mos6502->opcodes[0xDB] = OP_DB;
	mos6502->opcodes[0xDC] = OP_DC;
	mos6502->opcodes[0xDD] = CMP_ABSX;
	mos6502->opcodes[0xDE] = DEC_ABSX;
	mos6502->opcodes[0xDF] = OP_DF;
	//  MSD E
	mos6502->opcodes[0xE0] = CPX_IMM;
	mos6502->opcodes[0xE1] = SBC_INDX;
	mos6502->opcodes[0xE2] = OP_E2;
	mos6502->opcodes[0xE3] = OP_E3;
	mos6502->opcodes[0xE4] = CPX_ZP;
	mos6502->opcodes[0xE5] = SBC_ZP;
	mos6502->opcodes[0xE6] = INC_ZP;
	mos6502->opcodes[0xE7] = OP_E7;
	mos6502->opcodes[0xE8] = INX_IMP;
	mos6502->opcodes[0xE9] = SBC_IMM;
	mos6502->opcodes[0xEA] = NOP_IMP;
	mos6502->opcodes[0xEB] = OP_EB;
	mos6502->opcodes[0xEC] = CPX_ABS;
	mos6502->opcodes[0xED] = SBC_ABS;
	mos6502->opcodes[0xEE] = INC_ABS;
	mos6502->opcodes[0xEF] = OP_EF;
	//  MSD F
	mos6502->opcodes[0xF0] = BEQ_REL;
	mos6502->opcodes[0xF1] = SBC_INDY;
	mos6502->opcodes[0xF2] = OP_F2;
	mos6502->opcodes[0xF3] = OP_F3;
	mos6502->opcodes[0xF4] = OP_F4;
	mos6502->opcodes[0xF5] = SBC_ZPX;
	mos6502->opcodes[0xF6] = INC_ZPX;
	mos6502->opcodes[0xF7] = OP_F7;
	mos6502->opcodes[0xF8] = SED_IMP;
	mos6502->opcodes[0xF9] = SBC_ABSY;
	mos6502->opcodes[0xFA] = OP_FA;
	mos6502->opcodes[0xFB] = OP_FB;
	mos6502->opcodes[0xFC] = OP_FC;
	mos6502->opcodes[0xFD] = SBC_ABSX;
	mos6502->opcodes[0xFE] = INC_ABSX;
	mos6502->opcodes[0xFF] = OP_FF;
}
