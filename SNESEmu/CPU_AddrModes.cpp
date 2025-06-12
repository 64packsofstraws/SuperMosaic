#include "CPU.h"

void CPU::IMP()
{
	tick_components(6);
}

void CPU::IMM()
{
	if (GET_M()) {
		val = read8(FULL_PC);
		PC++;
	}
	else {
		val = read16(FULL_PC);
		PC += 2;
	}
}

void CPU::IMM8()
{
	val = read8(FULL_PC);
	PC++;
}

void CPU::IMM16()
{
	val = read16(FULL_PC);
	PC += 2;
}

void CPU::IMMX()
{
	if (GET_X()) {
		val = read8(FULL_PC);
		PC++;
	}
	else {
		val = read16(FULL_PC);
		PC += 2;
	}
}

void CPU::ACC()
{
}

void CPU::DP()
{
	addr = read8(FULL_PC);
	addr += D;
	addr &= 0xFFFF;

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::DPI()
{
	uint16_t direct_addr = read8(FULL_PC);
	direct_addr += D;

	addr = (DBR << 16) | read16(direct_addr);

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::DPIL()
{
	uint16_t direct_addr = read8(FULL_PC);
	direct_addr += D;

	uint8_t bank = read8(direct_addr + 2);
	addr = (bank << 16) | read16(direct_addr);

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::DPX()
{
	uint16_t m = !GET_X() ? X : X & 0xFF;
	addr = read8(FULL_PC) + D + m;
	addr &= 0xFFFF;

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::DPY()
{
	uint16_t m = !GET_X() ? Y : Y & 0xFF;
	addr = read8(FULL_PC) + D + m;
	addr &= 0xFFFF;

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::DPIX()
{
	uint16_t m = !GET_X() ? X : X & 0xFF;

	uint16_t direct_addr = read8(FULL_PC);
	direct_addr += D;

	uint32_t eff_addr = read16((direct_addr + m) & 0xFFFF);

	addr = (DBR << 16) | eff_addr;

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::DPIY()
{
	uint16_t m = !GET_X() ? Y : Y & 0xFF;

	uint16_t direct_addr = read8(FULL_PC);
	direct_addr += D;

	uint32_t eff_addr = (DBR << 16) | read16(direct_addr);

	addr = (eff_addr + m) & 0xFFFFFF;

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::DPILY()
{
	uint16_t m = !GET_X() ? Y : Y & 0xFF;

	uint16_t direct_addr = read8(FULL_PC);
	direct_addr += D;

	uint8_t bank = read8(direct_addr + 2);
	uint32_t base_addr = (bank << 16) | read16(direct_addr);

	bool page_cross = (base_addr & 0xFF00) != ((base_addr + m) & 0xFF00);
	addr = (base_addr + m) & 0xFFFFFF;

	PC++;

	tick_components(6 * ((D & 0xFF) != 0));
}

void CPU::REL()
{
	val = read8(FULL_PC);
	PC++;
}

void CPU::RELL()
{
	val = read16(FULL_PC);
	PC += 2;
}

void CPU::ABS()
{
	addr = (DBR << 16) | read16(FULL_PC);
	PC += 2;
}

void CPU::ABSL()
{
	uint8_t bank = read8(FULL_PC + 2);
	addr = (bank << 16) | read16(FULL_PC);
	PC += 3;
}

void CPU::ABSX()
{
	uint16_t m = !GET_X() ? X : X & 0xFF;

	uint32_t fetched_addr = (DBR << 16) | read16(FULL_PC);
	addr = (fetched_addr + m) & 0xFFFFFF;
	PC += 2;

	bool page_cross = (addr & 0xFF00) != (fetched_addr & 0xFF00);

	tick_components(6 * page_cross);
}

void CPU::ABSLX()
{
	uint16_t m = !GET_X() ? X : X & 0xFF;

	uint8_t bank = read8(FULL_PC + 2);
	uint32_t fetched_addr = (bank << 16) | read16(FULL_PC);
	PC += 3;

	addr = (fetched_addr + m) & 0xFFFFFF;
}

void CPU::ABSY()
{
	uint16_t m = !GET_X() ? Y : Y & 0xFF;

	uint32_t fetched_addr = (DBR << 16) | read16(FULL_PC);
	addr = (fetched_addr + m) & 0xFFFFFF;
	PC += 2;

	bool page_cross = (addr & 0xFF00) != (fetched_addr & 0xFF00);

	tick_components(6 * page_cross);
}

void CPU::ABSI()
{
	addr = read16(read16(FULL_PC));

	PC += 2;
}

void CPU::ABSIL()
{	
	uint16_t fetched_addr = read16(FULL_PC);
	uint8_t bank = read16((fetched_addr + 2) & 0xFFFF);

	addr = (bank << 16) | read16(fetched_addr);

	PC += 2;
}

void CPU::ABSIX()
{
	uint16_t m = GET_X() ? X & 0xFF : X;
	uint32_t fetched_addr = (((PBR << 16) | read16(FULL_PC)) + m) & 0xFFFFFF;

	addr = (PBR << 16) | read16(fetched_addr);

	PC += 2;
}

void CPU::SR()
{
	addr = (read8(FULL_PC) + SP) & 0xFFFF;
	PC++;
	tick_components(6);
}

void CPU::SRY()
{
	addr = (DBR << 16) | read16((read8(FULL_PC) + SP) & 0xFFFF);
	addr += !GET_X() ? Y : Y & 0xFF;

	addr &= 0xFFFFFF;
	PC++;

	tick_components(6);
}

void CPU::MOVE()
{
}
