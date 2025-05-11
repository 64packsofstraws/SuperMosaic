#include "CPU.h"

#define SET_C(c) (c) ? status |= 0x1 : status &= ~0x1;
#define SET_Z(c) (c) ? status |= 0x2 : status &= ~0x2; 
#define SET_I(c) (c) ? status |= 0x4 : status &= ~0x4; 
#define SET_D(c) (c) ? status |= 0x8 : status &= ~0x8;
#define SET_X(c) (c) ? status |= 0x10 : status &= ~0x10;
#define SET_M(c) (c) ? status |= 0x20 : status &= ~0x20;
#define SET_V(c) (c) ? status |= 0x40 : status &= ~0x40;
#define SET_N(c) (c) ? status |= 0x80 : status &= ~0x80;

#define GET_C() ((status >> 0) & 1)
#define GET_Z() ((status >> 1) & 1)
#define GET_I() ((status >> 2) & 1)
#define GET_D() ((status >> 3) & 1)
#define GET_X() ((status >> 4) & 1)
#define GET_M() ((status >> 5) & 1)
#define GET_V() ((status >> 6) & 1)
#define GET_N() ((status >> 7) & 1)


CPU::CPU() : memory(0x1000000, 0)
{
	A = X = Y;
	D = 0;
	DBR = 0;
	SP = 0x1FFF;
	status = 0;
	E = true;
}

uint8_t CPU::cpu_read(uint16_t addr)
{
	return memory[addr];
}

void CPU::cpu_write(uint16_t addr, uint8_t val)
{
	memory[addr] = val;
}


void CPU::check_addr_mode()
{

}

void CPU::push8(uint8_t v)
{
	cpu_write(0x100 + SP, v);
	SP--;
}

void CPU::push16(uint16_t v)
{
	cpu_write(0x100 + SP, v >> 8);
	SP--;

	cpu_write(0x100 + SP, v & 0xFF);
	SP--;
}

uint8_t CPU::pop8()
{
	SP++;
	return cpu_read(0x100 + SP);
}

uint16_t CPU::pop16()
{
	uint16_t v = 0;

	SP++;
	v |= cpu_read(0x100 + SP);
	SP++;
	v |= cpu_read(0x100 + SP) << 8;

	return v;
}

uint8_t CPU::IMP()
{
	return 0;
}

uint8_t CPU::IMM()
{
	val = GET_M() ? cpu_read(PC) : (cpu_read(PC + 1) << 8) | cpu_read(PC);
	PC++;

	return !GET_M();
}

uint8_t CPU::ACC()
{
	return 0;
}

uint8_t CPU::DP()
{
	addr = cpu_read(PC);
	addr += D;
	addr &= 0xFFFF;

	PC++;

	return (D & 0xFF) != 0;
}

uint8_t CPU::DPI()
{
	uint16_t direct_addr = cpu_read(PC);
	direct_addr += D;

	uint32_t eff_addr = cpu_read(direct_addr + 1) << 8 | cpu_read(direct_addr);

	addr = eff_addr;
	DBR = 0;

	PC++;

	return (D & 0xFF) != 0;
}

uint8_t CPU::DPIL()
{
	uint16_t direct_addr = cpu_read(PC);
	direct_addr += D;

	uint8_t bank = cpu_read(direct_addr + 2);
	uint32_t eff_addr = cpu_read(direct_addr + 1) << 8 | cpu_read(direct_addr);

	addr = (bank << 16) | eff_addr;
	DBR = 0;

	PC++;

	return (D & 0xFF) != 0;
	return 0;
}

uint8_t CPU::DPX()
{
	uint16_t m = !GET_M() ? X : X & 0xFF;
	addr = cpu_read(PC);
	addr += D + m;

	PC++;

	return !GET_M();
}

uint8_t CPU::DPY()
{
	uint16_t m = !GET_M() ? Y : Y & 0xFF;
	addr = cpu_read(PC);
	addr += D + m;

	PC++;

	return !GET_M();
}

uint8_t CPU::REL()
{
	val = cpu_read(PC);
	PC++;

	return 0;
}

uint8_t CPU::ABS()
{
	addr = (cpu_read(PC + 1) << 8) | cpu_read(PC);
	PC += 2;

	return !GET_M();
}

uint8_t CPU::ABSL()
{
	uint8_t bank = cpu_read(PC + 2);
	addr = (cpu_read(PC + 1) << 8) | cpu_read(PC);
	PC += 3;

	addr |= bank << 16;
	
	return !GET_M();
}

uint8_t CPU::ABSX()
{
	uint16_t m = !GET_M() ? X : X & 0xFF;

	uint16_t fetched_addr = (cpu_read(PC + 1) << 8) | cpu_read(PC);
	addr = fetched_addr + m;
	PC += 2;

	bool page_cross = (addr & 0xFF00) != (fetched_addr & 0xFF00);

	return page_cross + (!GET_M());
}

uint8_t CPU::ABSLX()
{
	uint16_t m = !GET_M() ? X : X & 0xFF;

	uint8_t bank = cpu_read(PC + 2);
	uint16_t fetched_addr = (cpu_read(PC + 1) << 8) | cpu_read(PC);
	
	addr = (bank << 16) | fetched_addr;
	addr += m;

	PC += 3;

	bool page_cross = (addr & 0xFF00) != (fetched_addr & 0xFF00);

	return page_cross + (!GET_M());
}

uint8_t CPU::ABSY()
{
	uint16_t m = !GET_M() ? Y : Y & 0xFF;

	uint16_t fetched_addr = (cpu_read(PC + 1) << 8) | cpu_read(PC);
	addr = fetched_addr + m;
	PC += 2;

	bool page_cross = (addr & 0xFF00) != (fetched_addr & 0xFF00);

	return page_cross + (!GET_M());
}

uint8_t CPU::ADC()
{
	check_addr_mode();

	uint16_t sum = A + val + GET_C();

	C = sum > 0xFF;
	Z = (sum & 0xFF) == 0;
	V = (~(A ^ sum) & (val ^ sum) & 0x80);
	N = (sum & 0x80) != 0;

	A = sum & 0xFF;

	return 1;
}

uint8_t CPU::AND()
{
	check_addr_mode();

	A &= val;

	Z = A == 0;
	N = (A & 0x80) != 0;

	return 1;
}

uint8_t CPU::ASL()
{
	// TODO
	if (0) {
		C = (A & 0x80) != 0;
		A <<= 1;

		Z = A == 0;
		N = (A & 0x80) != 0;
	}
	else {
		val = cpu_read(addr);

		C = (val & 0x80) != 0;
		val <<= 1;

		Z = val == 0;
		N = (val & 0x80) != 0;

		cpu_write(addr, val);
	}

	return 0;
}

uint8_t CPU::BCC()
{
	uint8_t bcycle = 0;

	if (!C) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BCS()
{
	uint8_t bcycle = 0;

	if (C) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BEQ()
{
	uint8_t bcycle = 0;

	if (Z) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BIT()
{
	uint8_t m = cpu_read(addr);
	uint8_t res = A & m;

	Z = res == 0;
	V = (m & 0x40) != 0;
	N = (m & 0x80) != 0;
	return 0;
}

uint8_t CPU::BMI()
{
	uint8_t bcycle = 0;

	if (N) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BNE()
{
	uint8_t bcycle = 0;

	if (!Z) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BPL()
{
	uint8_t bcycle = 0;

	if (!N) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BRK()
{
	uint8_t status = 0;

	status |= (C << 0);
	status |= (Z << 1);
	status |= (I << 2);
	status |= (D << 3);

	status |= (1 << 4);
	status |= (1 << 5);
	status |= (V << 6);
	status |= (N << 7);
	I = 1;
	PC--;
	push16(PC + 2);
	push8(status);
	PC = (cpu_read(0xFFFF) << 8) | cpu_read(0xFFFE);
	return 0;
}

uint8_t CPU::BVC()
{
	uint8_t bcycle = 0;

	if (!V) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BVS()
{
	uint8_t bcycle = 0;

	if (V) {
		int8_t casted_val = static_cast<int8_t>(val);
		bcycle += 1 + ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::CLC()
{
	C = 0;
	return 0;
}

uint8_t CPU::CLD()
{
	D = 0;
	return 0;
}

uint8_t CPU::CLI()
{
	I = 0;
	return 0;
}

uint8_t CPU::CLV()
{
	V = 0;
	return 0;
}

uint8_t CPU::CMP()
{
	check_addr_mode();

	uint8_t res = A - val;

	C = A >= val;
	Z = A == val;
	N = (res & 0x80) != 0;
	return 1;
}

uint8_t CPU::CPX()
{
	check_addr_mode();

	uint8_t res = X - val;

	C = X >= val;
	Z = X == val;
	N = (res & 0x80) != 0;
	return 0;
}

uint8_t CPU::CPY()
{
	check_addr_mode();

	uint8_t res = Y - val;

	C = Y >= val;
	Z = Y == val;
	N = (res & 0x80) != 0;
	return 0;
}

uint8_t CPU::DEC()
{
	uint8_t m = cpu_read(addr);
	m--;

	Z = m == 0;
	N = (m & 0x80) != 0;

	cpu_write(addr, m);
	return 0;
}

uint8_t CPU::DEX()
{
	X--;

	Z = X == 0;
	N = (X & 0x80) != 0;
	return 0;
}

uint8_t CPU::DEY()
{
	Y--;

	Z = Y == 0;
	N = (Y & 0x80) != 0;
	return 0;
}

uint8_t CPU::EOR()
{
	check_addr_mode();

	A ^= val;

	Z = A == 0;
	N = (A & 0x80) != 0;
	return 1;
}

uint8_t CPU::INC()
{
	uint8_t m = cpu_read(addr);
	m++;

	Z = m == 0;
	N = (m & 0x80) != 0;

	cpu_write(addr, m);
	return 0;
}

uint8_t CPU::INX()
{
	X++;

	Z = X == 0;
	N = (X & 0x80) != 0;
	return 0;
}

uint8_t CPU::INY()
{
	Y++;

	Z = Y == 0;
	N = (Y & 0x80) != 0;
	return 0;
}

uint8_t CPU::JMP()
{
	PC = addr;
	return 0;
}

uint8_t CPU::JSR()
{
	push16(PC - 1);
	PC = addr;
	return 0;
}

uint8_t CPU::LDA()
{
	check_addr_mode();

	A = val;

	Z = A == 0;
	N = (A & 0x80) != 0;
	return 1;
}

uint8_t CPU::LDX()
{
	check_addr_mode();

	X = val;

	Z = X == 0;
	N = (X & 0x80) != 0;
	return 1;
}

uint8_t CPU::LDY()
{
	check_addr_mode();

	Y = val;

	Z = Y == 0;
	N = (Y & 0x80) != 0;
	return 1;
}

uint8_t CPU::LSR()
{
	if (ins_table[opcode].addr_mode == &CPU::ACC) {
		C = A & 1;
		A >>= 1;

		Z = A == 0;
		N = (A & 0x80) != 0;
	}
	else {
		val = cpu_read(addr);

		C = val & 1;
		val >>= 1;

		Z = val == 0;
		N = (val & 0x80) != 0;

		cpu_write(addr, val);
	}

	return 0;
}

uint8_t CPU::NOP()
{
	return 0;
}

uint8_t CPU::ORA()
{
	check_addr_mode();

	A |= val;

	Z = A == 0;
	N = (A & 0x80) != 0;
	return 1;
}

uint8_t CPU::PHA()
{
	push8(A);
	return 0;
}

uint8_t CPU::PHP()
{
	uint8_t status = 0;

	status |= (C << 0);
	status |= (Z << 1);
	status |= (I << 2);
	status |= (D << 3);

	status |= (1 << 4);
	status |= (1 << 5);
	status |= (V << 6);
	status |= (N << 7);

	push8(status);
	return 0;
}

uint8_t CPU::PLA()
{
	A = pop8();

	Z = A == 0;
	N = (A & 0x80) != 0;
	return 0;
}

uint8_t CPU::PLP()
{
	uint8_t status = pop8();

	C = (status >> 0) & 1;
	Z = (status >> 1) & 1;
	I = (status >> 2) & 1;
	D = (status >> 3) & 1;
	V = (status >> 6) & 1;
	N = (status >> 7) & 1;
	return 0;
}

uint8_t CPU::ROL()
{
	if (ins_table[opcode].addr_mode == &CPU::ACC) {
		bool prev_bit = (A & 0x80) != 0;

		A <<= 1;
		A |= C;

		Z = A == 0;
		C = prev_bit;

		N = (A & 0x80) != 0;
	}
	else {
		val = cpu_read(addr);

		bool prev_bit = (val & 0x80) != 0;

		val <<= 1;
		val |= C;

		Z = val == 0;
		C = prev_bit;

		N = (val & 0x80) != 0;

		cpu_write(addr, val);
	}

	return 0;
}

uint8_t CPU::ROR()
{
	if (ins_table[opcode].addr_mode == &CPU::ACC) {
		bool prev_bit = A & 1;

		A >>= 1;
		A |= (C << 7);
		C = prev_bit;

		Z = A == 0;
		N = (A & 0x80) != 0;
	}
	else {
		val = cpu_read(addr);

		bool prev_bit = A & 1;

		val >>= 1;
		val |= (C << 7);
		C = prev_bit;

		Z = val == 0;
		N = (val & 0x80) != 0;

		cpu_write(addr, val);
	}
	return 0;
}

uint8_t CPU::RTI()
{
	uint8_t status = pop8();

	C = (status >> 0) & 1;
	Z = (status >> 1) & 1;
	I = (status >> 2) & 1;
	D = (status >> 3) & 1;
	V = (status >> 6) & 1;
	N = (status >> 7) & 1;

	PC = pop16();
	return 0;
}

uint8_t CPU::RTS()
{
	PC = pop16() + 1;
	return 0;
}

uint8_t CPU::SBC()
{
	check_addr_mode();

	uint16_t sum = A + (~val) + C;

	C = sum > 0xFF;
	Z = (sum & 0xFF) == 0;
	V = ((A ^ sum) & (val ^ sum) & 0x80);
	N = (sum & 0x80) != 0;

	A = sum & 0xFF;

	return 1;
}

uint8_t CPU::SEC()
{
	C = 1;
	return 0;
}

uint8_t CPU::SED()
{
	D = 1;
	return 0;
}

uint8_t CPU::SEI()
{
	I = 1;
	return 0;
}

uint8_t CPU::STA()
{
	cpu_write(addr, A);
	return 0;
}

uint8_t CPU::STX()
{
	cpu_write(addr, X);
	return 0;
}

uint8_t CPU::STY()
{
	cpu_write(addr, Y);
	return 0;
}

uint8_t CPU::TAX()
{
	X = A;

	Z = X == 0;
	N = (X & 0x80) != 0;
	return 0;
}

uint8_t CPU::TAY()
{
	Y = A;

	Z = Y == 0;
	N = (Y & 0x80) != 0;
	return 0;
}

uint8_t CPU::TSX()
{
	X = SP;

	Z = X == 0;
	N = (X & 0x80) != 0;
	return 0;
}

uint8_t CPU::TXA()
{
	A = X;

	Z = A == 0;
	N = (A & 0x80) != 0;
	return 0;
}

uint8_t CPU::TXS()
{
	SP = X;
	return 0;
}

uint8_t CPU::TYA()
{
	A = Y;

	Z = A == 0;
	N = (A & 0x80) != 0;
	return 0;
}


