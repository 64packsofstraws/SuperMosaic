#include "CPU.h"

uint8_t CPU::ADC() {
	check_addr_mode();

	if (GET_M()) {
		uint8_t a8 = A & 0xFF;
		uint16_t sum;

		if (GET_D()) {
			sum = (a8 & 0xF) + (val & 0xF) + GET_C();
			if (sum > 0x9) sum += 0x6;

			SET_C(sum > 0xF);
			sum = (a8 & 0xF0) + (val & 0xF0) + (GET_C() << 4) + (sum & 0xF);
		}
		else {
			sum = a8 + val + GET_C();
		}

		SET_V(~(a8 ^ val) & (a8 ^ sum) & 0x80);

		if (GET_D() && sum > 0x9F) sum += 0x60;

		SET_C(sum > 0xFF);
		SET_Z((sum & 0xFF) == 0);
		SET_N((sum & 0x80) != 0);

		A = (A & 0xFF00) | sum & 0xFF;
	}
	else {
		uint32_t sum;

		if (GET_D()) {
			sum = (A & 0xF) + (val & 0xF) + GET_C();

			if (sum > 0x9) sum += 0x6;
			SET_C(sum > 0xF);

			sum = (A & 0xF0) + (val & 0xF0) + (GET_C() << 4) + (sum & 0xF);

			if (sum > 0x9F) sum += 0x60;
			SET_C(sum > 0xFF);

			sum = (A & 0xF00) + (val & 0xF00) + (GET_C() << 8) + (sum & 0xFF);

			if (sum > 0x9FF) sum += 0x600;
			SET_C(sum > 0xFFF);

			sum = (A & 0xF000) + (val & 0xF000) + (GET_C() << 12) + (sum & 0xFFF);
		}
		else {
			sum = A + val + GET_C();
		}

		SET_V((~(A ^ val) & (A ^ sum) & 0x8000));

		if (GET_D() && sum > 0x9FFF) {
			sum += 0x6000;
		}

		SET_C(sum > 0xFFFF);
		SET_Z((sum & 0xFFFF) == 0);
		SET_N((sum & 0x8000) != 0);

		A = sum & 0xFFFF;
	}

	return 1;
}

uint8_t CPU::AND()
{
	check_addr_mode();

	if (GET_M()) {
		uint8_t res = A & val;
		A = (A & 0xFF00) | res;

		SET_Z(res == 0);
		SET_N((res & 0x80) != 0);
	}
	else {
		A &= val;

		SET_Z(A == 0);
		SET_N((A & 0x8000) != 0);
	}

	return 1;
}

uint8_t CPU::ASL()
{
	if (GET_M()) {
		uint8_t tmp = read8(addr);

		SET_C((tmp & 0x80) != 0);
		tmp <<= 1;

		SET_Z(tmp == 0);
		SET_N((tmp & 0x80) != 0);

		write8(addr, tmp);
	}
	else {
		uint16_t tmp = read16(addr);

		SET_C((tmp & 0x8000) != 0);
		tmp <<= 1;

		SET_Z(tmp == 0);
		SET_N((tmp & 0x8000) != 0);

		write16(addr, tmp);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::ASLA()
{
	if (GET_M()) {
		uint8_t result = A & 0xFF;

		SET_C((result & 0x80) != 0);
		result <<= 1;

		SET_Z(result == 0);
		SET_N((result & 0x80) != 0);

		A = (A & 0xFF00) | result;
	}
	else {
		SET_C((A & 0x8000) != 0);
		A <<= 1;

		SET_Z(A == 0);
		SET_N((A & 0x8000) != 0);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::BCC()
{
	uint8_t bcycle = 0;

	if (!GET_C()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BCS()
{
	uint8_t bcycle = 0;

	if (GET_C()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BEQ()
{
	uint8_t bcycle = 0;

	if (GET_Z()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BIT()
{
	if (GET_M()) {
		uint8_t tmp = read8(addr);
		uint8_t result = (A & 0xFF) & tmp;

		SET_Z(result == 0);
		SET_V(tmp & 0x40);
		SET_N(tmp & 0x80);
	}
	else {
		uint16_t tmp = read16(addr);
		uint16_t result = A & tmp;

		SET_Z(result == 0);
		SET_V(tmp & 0x4000);
		SET_N(tmp & 0x8000);
	}
	return 0;
}

uint8_t CPU::BIT_IMM()
{
	if (GET_M()) {
		uint8_t result = (A & 0xFF) & val;
		SET_Z(result == 0);
	}
	else {
		uint16_t result = A & val;
		SET_Z(result == 0);
	}
	return 0;
}

uint8_t CPU::BMI()
{
	uint8_t bcycle = 0;

	if (GET_N()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BNE()
{
	uint8_t bcycle = 0;

	if (!GET_Z()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BPL()
{
	uint8_t bcycle = 0;

	if (!GET_N()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BRA()
{
	PC += static_cast<int8_t>(val);
	return 0;
}

uint8_t CPU::BRL()
{
	PC += static_cast<int16_t>(val);
	return E;
}

uint8_t CPU::BRK()
{
	PC++;
	push8(PBR);
	push16(PC);

	push8(status);
	SET_I(1);
	SET_D(0);

	PBR = 0;
	PC = read16(0xFFE6);

	return 0;
}

uint8_t CPU::BVC()
{
	uint8_t bcycle = 0;

	if (!GET_V()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::BVS()
{
	uint8_t bcycle = 0;

	if (GET_V()) {
		int8_t casted_val = static_cast<int8_t>(val);
		tick_components(6 + 6 * ((PC & 0xFF00) != ((PC + casted_val) & 0xFF00)));

		PC += casted_val;
	}

	return bcycle;
}

uint8_t CPU::CLC()
{
	SET_C(0);
	return 0;
}

uint8_t CPU::CLD()
{
	SET_D(0);
	return 0;
}

uint8_t CPU::CLI()
{
	SET_I(0);
	return 0;
}

uint8_t CPU::CLV()
{
	SET_V(0);
	return 0;
}

uint8_t CPU::CMP()
{
	check_addr_mode();

	if (GET_M()) {
		uint8_t a8 = A & 0xFF;
		uint8_t result = a8 - val;

		SET_C(a8 >= val);
		SET_Z(a8 == val);
		SET_N(result & 0x80);
	}
	else {
		uint16_t result = A - val;

		SET_C(A >= val);
		SET_Z(A == val);
		SET_N(result & 0x8000);
	}
	return 1;
}

uint8_t CPU::COP()
{
	PC++;
	push8(PBR);
	push16(PC);

	push8(status);
	SET_I(1);
	SET_D(0);

	PBR = 0;
	PC = read16(0xFFE4);
	return 0;
}

uint8_t CPU::CPX()
{
	check_addr_modex();

	if (GET_X()) {
		uint8_t x8 = X & 0xFF;
		uint8_t result = x8 - val;

		SET_C(x8 >= val);
		SET_Z(x8 == val);
		SET_N(result & 0x80);
	}
	else {
		uint16_t result = X - val;

		SET_C(X >= val);
		SET_Z(X == val);
		SET_N(result & 0x8000);
	}

	return 1;
}

uint8_t CPU::CPY()
{
	check_addr_modex();

	if (GET_X()) {
		uint8_t y8 = Y & 0xFF;
		uint8_t result = y8 - val;

		SET_C(y8 >= val);
		SET_Z(y8 == val);
		SET_N(result & 0x80);
	}
	else {
		uint16_t result = Y - val;

		SET_C(Y >= val);
		SET_Z(Y == val);
		SET_N(result & 0x8000);
	}

	return 1;
}

uint8_t CPU::DEC()
{
	if (GET_M()) {
		uint8_t res = read8(addr) - 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		write8(addr, res);
	}
	else {
		uint16_t res = read16(addr) - 1;

		SET_Z(res == 0);
		SET_N(res & 0x8000);

		write16(addr, res);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::DEA()
{
	if (GET_M()) {
		uint8_t res = (A & 0xFF) - 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		A = (A & 0xFF00) | res;
	}
	else {
		A--;

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	return 0;
}

uint8_t CPU::DEX()
{
	if (GET_X()) {
		uint8_t res = (X & 0xFF) - 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		X = (X & 0xFF00) | res;
	}
	else {
		uint16_t res = X - 1;

		SET_Z(res == 0);
		SET_N(res & 0x8000);

		X = res;
	}

	return 0;
}

uint8_t CPU::DEY()
{
	if (GET_X()) {
		uint8_t res = (Y & 0xFF) - 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		Y = (Y & 0xFF00) | res;
	}
	else {
		uint16_t res = Y - 1;

		SET_Z(res == 0);
		SET_N(res & 0x8000);

		Y = res;
	}

	return 0;
}

uint8_t CPU::EOR()
{
	check_addr_mode();

	if (GET_M()) {
		uint8_t res = A ^ val;
		A = (A & 0xFF00) | res;

		SET_Z(res == 0);
		SET_N((res & 0x80) != 0);
	}
	else {
		A ^= val;

		SET_Z(A == 0);
		SET_N((A & 0x8000) != 0);
	}

	return 1;
}

uint8_t CPU::INC()
{
	if (GET_M()) {
		uint8_t res = read8(addr) + 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		write8(addr, res);
	}
	else {
		uint16_t res = read16(addr) + 1;

		SET_Z(res == 0);
		SET_N(res & 0x8000);

		write16(addr, res);
	}

	return 1;
}

uint8_t CPU::INA()
{
	if (GET_M()) {
		uint8_t res = (A & 0xFF) + 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		A = (A & 0xFF00) | res;
	}
	else {
		A++;

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	return 0;
}

uint8_t CPU::INX()
{
	if (GET_M()) {
		uint8_t res = (X & 0xFF) + 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		X = (X & 0xFF00) | res;
	}
	else {
		X++;

		SET_Z(X == 0);
		SET_N(X & 0x8000);
	}

	return 0;
}

uint8_t CPU::INY()
{
	if (GET_M()) {
		uint8_t res = (Y & 0xFF) + 1;

		SET_Z(res == 0);
		SET_N(res & 0x80);

		Y = (Y & 0xFF00) | res;
	}
	else {
		Y++;

		SET_Z(Y == 0);
		SET_N(Y & 0x8000);
	}

	return 0;
}

uint8_t CPU::JMP()
{
	PC = addr;
	return 1;
}

uint8_t CPU::JML()
{
	PBR = (addr >> 16) & 0xFF;
	PC = addr & 0xFFFF;
	return 0;
}

uint8_t CPU::JSR()
{
	push16(PC - 1);
	PC = addr;
	return 0;
}

uint8_t CPU::JSRL()
{
	push8(PBR);
	push16(PC - 1);

	PBR = (addr >> 16) & 0xFF;
	PC = addr & 0xFFFF;
	return 0;
}

uint8_t CPU::LDA()
{
	check_addr_mode();

	if (GET_M()) {
		A = (A & 0xFF00) | val;

		SET_Z(val == 0);
		SET_N(val & 0x80);
	}
	else {
		A = val;

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	return 1;
}

uint8_t CPU::LDX()
{
	check_addr_modex();

	if (GET_X()) {
		X = (X & 0xFF00) | val;

		SET_Z(val == 0);
		SET_N(val & 0x80);
	}
	else {
		X = val;

		SET_Z(X == 0);
		SET_N(X & 0x8000);
	}

	return 1;
}

uint8_t CPU::LDY()
{
	check_addr_modex();

	if (GET_X()) {
		Y = (Y & 0xFF00) | val;

		SET_Z(val == 0);
		SET_N(val & 0x80);
	}
	else {
		Y = val;

		SET_Z(Y == 0);
		SET_N(Y & 0x8000);
	}

	return 1;
}

uint8_t CPU::LSR()
{
	if (GET_M()) {
		uint8_t tmp = read8(addr);

		SET_C(tmp & 1);
		tmp >>= 1;

		SET_Z(tmp == 0);
		SET_N(tmp & 0x80);

		write8(addr, tmp);
	}
	else {
		uint16_t tmp = read16(addr);

		SET_C(tmp & 1);
		tmp >>= 1;

		SET_Z(tmp == 0);
		SET_N(tmp & 0x8000);

		write16(addr, tmp);
	}

	tick_components(6);
	return 0;
}

uint8_t CPU::LSRA()
{
	if (GET_M()) {
		uint8_t a8 = A & 0xFF;

		SET_C(a8 & 1);
		a8 >>= 1;

		SET_Z(a8 == 0);
		SET_N(a8 & 0x80);

		A = (A & 0xFF00) | a8;
	}
	else {
		SET_C(A & 1);
		A >>= 1;

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	tick_components(6);
	return 0;
}

uint8_t CPU::MVN()
{
	uint8_t dest_bank = read8(FULL_PC);
	uint8_t src_bank = read8(FULL_PC + 1);
	PC += 2;
	DBR = dest_bank;

	if (GET_X()) {
		if (A == 0xFFFF) return 0;
		
		uint8_t x = X & 0xFF;
		uint8_t y = Y & 0xFF;

		write8(y++, read8(x++));
		
		A--;
		tick_components(12);

		X = (X & 0xFF00) | (x & 0xFF);
		Y = (Y & 0xFF00) | (y & 0xFF);
		
		PC -= 3;
	}
	else {
		if (A == 0xFFFF) return 0;

		write8((DBR << 16) | Y, read8((src_bank << 16) | X));
		Y++;
		X++;

		A--;
		tick_components(12);

		PC -= 3;
	}

	return 0;
}

uint8_t CPU::MVP() {
	uint8_t dest_bank = read8(FULL_PC);
	uint8_t src_bank = read8(FULL_PC + 1);
	DBR = dest_bank;

	if (GET_X()) {
		if (A == 0xFFFF) return 0;

		uint8_t x = X & 0xFF;
		uint8_t y = Y & 0xFF;

		write8(y--, read8(x--));

		A--;
		tick_components(12);

		X = (X & 0xFF00) | (x & 0xFF);
		Y = (Y & 0xFF00) | (y & 0xFF);

		PC -= 3;
	}
	else {
		if (A == 0xFFFF) return 0;

		write8((DBR << 16) | Y, read8((src_bank << 16) | X));
		Y--;
		X--;

		A--;
		tick_components(12);
		
		PC -= 3;
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

	if (GET_M()) {
		uint8_t res = A | val;
		A = (A & 0xFF00) | res;

		SET_Z(res == 0);
		SET_N((res & 0x80) != 0);
	}
	else {
		A |= val;

		SET_Z(A == 0);
		SET_N((A & 0x8000) != 0);
	}

	return 1;
}

uint8_t CPU::PEA()
{
	push16(addr);
	return 0;
}

uint8_t CPU::PEI()
{
	push16(addr);
	return 1;
}

uint8_t CPU::PER()
{
	push16(addr + PC);
	return 0;
}

uint8_t CPU::PHA()
{
	if (GET_M()) push8(A & 0xFF);
	else push16(A);
	return 1;
}

uint8_t CPU::PHB()
{
	push8(DBR);
	return 0;
}

uint8_t CPU::PHD()
{
	push16(D);
	return 0;
}

uint8_t CPU::PHK()
{
	push8(PBR);
	return 0;
}

uint8_t CPU::PHP()
{
	push8(status);
	return 0;
}

uint8_t CPU::PHX()
{
	if (GET_X()) push8(X & 0xFF);
	else push16(X);
	return 1;
}

uint8_t CPU::PHY()
{
	if (GET_X()) push8(Y & 0xFF);
	else push16(Y);
	return 1;
}

uint8_t CPU::PLA()
{
	if (GET_M()) {
		A = (A & 0xFF00) | pop8();
		SET_Z((A & 0xFF) == 0);
		SET_N(A & 0x80);
	}
	else {
		A = pop16();
		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::PLB()
{
	DBR = pop8();

	SET_Z(DBR == 0);
	SET_N(DBR & 0x80);
	tick_components(6);
	return 0;
}

uint8_t CPU::PLD()
{
	D = pop16();

	SET_Z(D == 0);
	SET_N(D & 0x8000);
	tick_components(6);
	return 0;
}

uint8_t CPU::PLP()
{
	status = pop8();

	if (GET_X()) {
		X &= 0xFF;
		Y &= 0xFF;
	}

	tick_components(6);
	return 0;
}

uint8_t CPU::PLX()
{
	if (GET_X()) {
		X = (X & 0xFF00) | pop8();
		SET_Z((X & 0xFF) == 0);
		SET_N(X & 0x80);
	}
	else {
		X = pop16();
		SET_Z(X == 0);
		SET_N(X & 0x8000);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::PLY()
{
	if (GET_X()) {
		Y = (Y & 0xFF00) | pop8();
		SET_Z((Y & 0xFF) == 0);
		SET_N(Y & 0x80);
	}
	else {
		Y = pop16();
		SET_Z(Y == 0);
		SET_N(Y & 0x8000);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::REP()
{
	status &= ~(val & 0xFF);
	tick_components(6);
	return 0;
}

uint8_t CPU::ROL()
{
	if (GET_M()) {
		uint8_t tmp = read8(addr);
		bool prev_bit = tmp >> 7;

		tmp <<= 1;
		tmp |= GET_C();

		SET_C(prev_bit);

		SET_Z(tmp == 0);
		SET_N(tmp & 0x80);

		write8(addr, tmp);
	}
	else {
		uint16_t tmp = read16(addr);
		bool prev_bit = tmp >> 15;

		tmp <<= 1;
		tmp |= GET_C();

		SET_C(prev_bit);

		SET_Z(tmp == 0);
		SET_N(tmp & 0x8000);

		write16(addr, tmp);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::ROLA()
{
	if (GET_M()) {
		uint8_t a8 = A & 0xFF;
		bool prev_bit = a8 >> 7;

		a8 <<= 1;
		a8 |= GET_C();

		SET_C(prev_bit);

		SET_Z(a8 == 0);
		SET_N(a8 & 0x80);

		A = (A & 0xFF00) | a8;
	}
	else {
		bool prev_bit = A >> 15;

		A <<= 1;
		A |= GET_C();

		SET_C(prev_bit);

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::ROR()
{
	if (GET_M()) {
		uint8_t tmp = read8(addr);
		bool prev_bit = tmp & 1;

		tmp >>= 1;
		tmp |= GET_C() << 7;

		SET_C(prev_bit);

		SET_Z(tmp == 0);
		SET_N(tmp & 0x80);

		write8(addr, tmp);
	}
	else {
		uint16_t tmp = read16(addr);
		bool prev_bit = tmp & 1;

		tmp >>= 1;
		tmp |= GET_C() << 15;

		SET_C(prev_bit);

		SET_Z(tmp == 0);
		SET_N(tmp & 0x8000);

		write16(addr, tmp);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::RORA()
{
	if (GET_M()) {
		uint8_t a8 = A & 0xFF;
		bool prev_bit = a8 & 1;

		a8 >>= 1;
		a8 |= GET_C() << 7;

		SET_C(prev_bit);

		SET_Z(a8 == 0);
		SET_N(a8 & 0x80);

		A = (A & 0xFF00) | a8;
	}
	else {
		bool prev_bit = A & 1;

		A >>= 1;
		A |= GET_C() << 15;

		SET_C(prev_bit);

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::RTI()
{
	status = pop8();
	PC = pop16();
	PBR = pop8();

	tick_components(6);
	return 0;
}

uint8_t CPU::RTL()
{
	PC = pop16() + 1;
	PBR = pop8();

	tick_components(6);
	return 0;
}

uint8_t CPU::RTS()
{
	PC = pop16() + 1;

	tick_components(12);
	return 0;
}

uint8_t CPU::SBC() {
	check_addr_mode();
	val = ~val;

	if (GET_M()) {
		uint8_t a8 = A & 0xFF;
		uint16_t sum;
		uint8_t data = val & 0xFF;

		if (GET_D()) {
			sum = (A & 0xF) + (data & 0xF) + GET_C();
			if(sum <= 0xF) sum -= 0x06;

			SET_C(sum > 0xF);
			sum = (A & 0xF0) + (data & 0xF0) + (GET_C() << 4) + (sum & 0xF);
		}
		else {
			sum = a8 + data + GET_C();
		}

		SET_V((~(a8 ^ data)) & (a8 ^ sum) & 0x80);

		if (GET_D() && sum <= 0xFF) sum -= 0x60;

		SET_C(sum > 0xFF);
		SET_Z((sum & 0xFF) == 0);
		SET_N((sum & 0x80) != 0);

		A = (A & 0xFF00) | sum & 0xFF;
	}
	else {
		uint32_t sum;

		if (GET_D()) {
			sum = (A & 0xF) + (val & 0xF) + GET_C();
			if(sum <= 0xF) sum -= 0x6;

			SET_C(sum > 0xF);
			
			sum = (A & 0xF0) + (val & 0xF0) + (GET_C() << 4) + (sum & 0xF);
			if(sum <= 0xFF) sum -= 0x60;
			
			SET_C(sum > 0xFF);
			
			sum = (A & 0xF00) + (val & 0xF00) + (GET_C() << 8) + (sum & 0xFF);
			if(sum <= 0xFFF) sum -= 0x600;
			
			SET_C(sum > 0xFFF);
			
			sum = (A & 0xF000) + (val & 0xF000) + (GET_C() << 12) + (sum & 0xFFF);
		}
		else {
			sum = A + val + GET_C();
		}

		SET_V((~(A ^ val)) & (A ^ sum) & 0x8000);

		if (GET_D() && sum <= 0xFFFF) sum -= 0x6000;

		SET_C(sum > 0xFFFF);
		SET_Z((sum & 0xFFFF) == 0);
		SET_N((sum & 0x8000) != 0);

		A = sum & 0xFFFF;
	}

	return 1;
}

uint8_t CPU::SEP()
{
	status |= (val & 0xFF);

	if (GET_X()) {
		X &= 0xFF;
		Y &= 0xFF;
	}

	tick_components(6);
	return 0;
}

uint8_t CPU::STP()
{
	PC--;
	tick_components(6);
	return 0;
}

uint8_t CPU::SEC()
{
	SET_C(1);
	return 0;
}

uint8_t CPU::SED()
{
	SET_D(1);
	return 0;
}

uint8_t CPU::SEI()
{
	SET_I(1);
	return 0;
}

uint8_t CPU::STA()
{
	if (GET_M()) {
		write8(addr, A & 0xFF);
	}
	else {
		write16(addr, A);
	}

	return 1;
}

uint8_t CPU::STX()
{
	if (GET_X()) {
		write8(addr, X & 0xFF);
	}
	else {
		write16(addr, X);
	}

	return 1;
}

uint8_t CPU::STY()
{
	if (GET_X()) {
		write8(addr, Y & 0xFF);
	}
	else {
		write16(addr, Y);
	}

	return 1;
}

uint8_t CPU::STZ()
{
	if (GET_M()) {
		write8(addr, 0);
	}
	else {
		write16(addr, 0);
	}

	return 1;
}

uint8_t CPU::TAX()
{
	if (GET_X()) {
		uint8_t a8 = A & 0xFF;
		X = a8;

		SET_Z(a8 == 0);
		SET_N(a8 & 0x80);
	}
	else {
		X = A;

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	return 0;
}

uint8_t CPU::TAY()
{
	if (GET_X()) {
		uint8_t a8 = A & 0xFF;
		Y = a8;

		SET_Z(a8 == 0);
		SET_N(a8 & 0x80);
	}
	else {
		Y = A;

		SET_Z(A == 0);
		SET_N(A & 0x8000);
	}

	return 0;
}

uint8_t CPU::TCD()
{
	D = A;

	SET_Z(D == 0);
	SET_N(D & 0x8000);
	return 0;
}

uint8_t CPU::TDC()
{
	A = D;

	SET_Z(A == 0);
	SET_N(A & 0x8000);
	return 0;
}

uint8_t CPU::TCS()
{
	SP = A;
	return 0;
}

uint8_t CPU::TSC()
{
	A = SP;

	SET_Z(A == 0);
	SET_N(A & 0x8000);
	return 0;
}

uint8_t CPU::TSX()
{
	if (GET_X()) {
		uint8_t sp8 = SP & 0xFF;
		X = sp8;

		SET_Z(sp8 == 0);
		SET_N(sp8 & 0x80);
	}
	else {
		X = SP;

		SET_Z(SP == 0);
		SET_N(SP & 0x8000);
	}
	return 0;
}

uint8_t CPU::TXA()
{
	if (GET_M()) {
		uint8_t x8 = X & 0xFF;
		A = (A & 0xFF00) | x8;

		SET_Z(x8 == 0);
		SET_N(x8 & 0x80);
	}
	else {
		A = X;

		SET_Z(X == 0);
		SET_N(X & 0x8000);
	}

	return 0;
}

uint8_t CPU::TXS()
{
	if (GET_X()) {
		SP = X & 0xFF;
	}
	else {
		SP = X;
	}

	return 0;
}

uint8_t CPU::TXY()
{
	if (GET_X()) {
		uint8_t x8 = X & 0xFF;
		Y = (Y & 0xFF00) | x8;

		SET_Z(Y == 0);
		SET_N(Y & 0x80);
	}
	else {
		Y = X;

		SET_Z(X == 0);
		SET_N(X & 0x8000);
	}

	return 0;
}

uint8_t CPU::TYA()
{
	if (GET_M()) {
		uint8_t y8 = Y & 0xFF;
		A = (A & 0xFF00) | y8;

		SET_Z(y8 == 0);
		SET_N(y8 & 0x80);
	}
	else {
		A = Y;

		SET_Z(Y == 0);
		SET_N(Y & 0x8000);
	}

	return 0;
}

uint8_t CPU::TYX()
{
	if (GET_X()) {
		uint8_t y8 = Y & 0xFF;
		X = (X & 0xFF00) | y8;

		SET_Z(y8 == 0);
		SET_N(y8 & 0x80);
	}
	else {
		X = Y;

		SET_Z(Y == 0);
		SET_N(Y & 0x8000);
	}

	return 0;
}

uint8_t CPU::TRB()
{
	if (GET_M()) {
		uint8_t a8 = A & 0xFF;
		uint8_t tmp = read8(addr);

		write8(addr, tmp & (~a8));

		SET_Z((a8 & tmp) == 0);
	}
	else {
		uint16_t tmp = read16(addr);

		write16(addr, tmp & (~A));

		SET_Z((A & tmp) == 0);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::TSB()
{
	if (GET_M()) {
		uint8_t a8 = A & 0xFF;
		uint8_t tmp = read8(addr);

		write8(addr, tmp | a8);

		SET_Z((a8 & tmp) == 0);
	}
	else {
		uint16_t tmp = read16(addr);

		write16(addr, tmp | A);

		SET_Z((A & tmp) == 0);
	}

	tick_components(6);
	return 1;
}

uint8_t CPU::WAI()
{
	tick_components(6);
	PC--;
	return 0;
}

uint8_t CPU::WDM()
{
	PC++;
	return 0;
}

uint8_t CPU::XBA()
{
	uint8_t lo = A & 0xFF;
	uint8_t hi = (A >> 8) & 0xFF;

	A = (lo << 8) | hi;

	SET_Z((A & 0xFF) == 0);
	SET_N(A & 0x80);

	tick_components(6);
	return 0;
}

uint8_t CPU::XCE()
{
	bool carry = GET_C();
	SET_C(E);
	E = carry;
	return 0;
}