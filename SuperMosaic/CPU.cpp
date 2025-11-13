#include "CPU.h"
#include "SNES.h"

CPU::CPU(SNES* snes) : snes(snes)
{
	ins_table[0x69] = { &CPU::ADC, &CPU::IMM, 2 };
	ins_table[0x6D] = { &CPU::ADC, &CPU::ABS, 4 };
	ins_table[0x6F] = { &CPU::ADC, &CPU::ABSL, 5 };
	ins_table[0x65] = { &CPU::ADC, &CPU::DP, 3 };
	ins_table[0x72] = { &CPU::ADC, &CPU::DPI, 5 };
	ins_table[0x67] = { &CPU::ADC, &CPU::DPIL, 6 };
	ins_table[0x7D] = { &CPU::ADC, &CPU::ABSX, 4 };
	ins_table[0x7F] = { &CPU::ADC, &CPU::ABSLX, 5 };
	ins_table[0x79] = { &CPU::ADC, &CPU::ABSY, 4 };
	ins_table[0x75] = { &CPU::ADC, &CPU::DPX, 4 };
	ins_table[0x61] = { &CPU::ADC, &CPU::DPIX, 6 };
	ins_table[0x71] = { &CPU::ADC, &CPU::DPIY, 5 };
	ins_table[0x77] = { &CPU::ADC, &CPU::DPILY, 6 };
	ins_table[0x63] = { &CPU::ADC, &CPU::SR, 4 };
	ins_table[0x73] = { &CPU::ADC, &CPU::SRY, 7 };

	ins_table[0x29] = { &CPU::AND, &CPU::IMM, 2 };
	ins_table[0x2D] = { &CPU::AND, &CPU::ABS, 4 };
	ins_table[0x2F] = { &CPU::AND, &CPU::ABSL, 5 };
	ins_table[0x25] = { &CPU::AND, &CPU::DP, 3 };
	ins_table[0x32] = { &CPU::AND, &CPU::DPI, 5 };
	ins_table[0x27] = { &CPU::AND, &CPU::DPIL, 6 };
	ins_table[0x3D] = { &CPU::AND, &CPU::ABSX, 4 };
	ins_table[0x3F] = { &CPU::AND, &CPU::ABSLX, 5 };
	ins_table[0x39] = { &CPU::AND, &CPU::ABSY, 4 };
	ins_table[0x35] = { &CPU::AND, &CPU::DPX, 4 };
	ins_table[0x21] = { &CPU::AND, &CPU::DPIX, 6 };
	ins_table[0x31] = { &CPU::AND, &CPU::DPIY, 5 };
	ins_table[0x37] = { &CPU::AND, &CPU::DPILY, 6 };
	ins_table[0x23] = { &CPU::AND, &CPU::SR, 4 };
	ins_table[0x33] = { &CPU::AND, &CPU::SRY, 7 };

	ins_table[0x0A] = { &CPU::ASLA, &CPU::ACC, 2 };
	ins_table[0x0E] = { &CPU::ASL, &CPU::ABS, 6 };
	ins_table[0x06] = { &CPU::ASL, &CPU::DP, 5 };
	ins_table[0x1E] = { &CPU::ASL, &CPU::ABSX, 7 };
	ins_table[0x16] = { &CPU::ASL, &CPU::DPX, 6 };

	ins_table[0x90] = { &CPU::BCC, &CPU::REL, 2 };
	ins_table[0xB0] = { &CPU::BCS, &CPU::REL, 2 };
	ins_table[0xF0] = { &CPU::BEQ, &CPU::REL, 2 };
	ins_table[0xD0] = { &CPU::BNE, &CPU::REL, 2 };
	ins_table[0x30] = { &CPU::BMI, &CPU::REL, 2 };
	ins_table[0x10] = { &CPU::BPL, &CPU::REL, 2 };
	ins_table[0x50] = { &CPU::BVC, &CPU::REL, 2 };
	ins_table[0x70] = { &CPU::BVS, &CPU::REL, 2 };
	ins_table[0x80] = { &CPU::BRA, &CPU::REL, 3 };
	ins_table[0x82] = { &CPU::BRL, &CPU::RELL, 4 };

	ins_table[0x89] = { &CPU::BIT_IMM, &CPU::IMM, 2 };
	ins_table[0x2C] = { &CPU::BIT, &CPU::ABS, 4 };
	ins_table[0x24] = { &CPU::BIT, &CPU::DP, 3 };
	ins_table[0x3C] = { &CPU::BIT, &CPU::ABSX, 4 };
	ins_table[0x34] = { &CPU::BIT, &CPU::DPX, 4 };

	ins_table[0x00] = { &CPU::BRK, &CPU::IMP, 7 };

	ins_table[0x18] = { &CPU::CLC, &CPU::IMP, 2 };
	ins_table[0xD8] = { &CPU::CLD, &CPU::IMP, 2 };
	ins_table[0x58] = { &CPU::CLI, &CPU::IMP, 2 };
	ins_table[0xB8] = { &CPU::CLV, &CPU::IMP, 2 };

	ins_table[0xC9] = { &CPU::CMP, &CPU::IMM, 2 };
	ins_table[0xCD] = { &CPU::CMP, &CPU::ABS, 4 };
	ins_table[0xCF] = { &CPU::CMP, &CPU::ABSL, 5 };
	ins_table[0xC5] = { &CPU::CMP, &CPU::DP, 3 };
	ins_table[0xD2] = { &CPU::CMP, &CPU::DPI, 5 };
	ins_table[0xC7] = { &CPU::CMP, &CPU::DPIL, 6 };
	ins_table[0xDD] = { &CPU::CMP, &CPU::ABSX, 4 };
	ins_table[0xDF] = { &CPU::CMP, &CPU::ABSLX, 5 };
	ins_table[0xD9] = { &CPU::CMP, &CPU::ABSY, 4 };
	ins_table[0xD5] = { &CPU::CMP, &CPU::DPX, 4 };
	ins_table[0xC1] = { &CPU::CMP, &CPU::DPIX, 6 };
	ins_table[0xD1] = { &CPU::CMP, &CPU::DPIY, 5 };
	ins_table[0xD7] = { &CPU::CMP, &CPU::DPILY, 6 };
	ins_table[0xC3] = { &CPU::CMP, &CPU::SR, 4 };
	ins_table[0xD3] = { &CPU::CMP, &CPU::SRY, 7 };

	ins_table[0x02] = { &CPU::COP, &CPU::IMP, 7 };

	ins_table[0xE0] = { &CPU::CPX, &CPU::IMMX, 2 };
	ins_table[0xEC] = { &CPU::CPX, &CPU::ABS, 4 };
	ins_table[0xE4] = { &CPU::CPX, &CPU::DP, 3 };

	ins_table[0xC0] = { &CPU::CPY, &CPU::IMMX, 2 };
	ins_table[0xCC] = { &CPU::CPY, &CPU::ABS, 4 };
	ins_table[0xC4] = { &CPU::CPY, &CPU::DP, 3 };

	ins_table[0x3A] = { &CPU::DEA, &CPU::ACC, 2 };
	ins_table[0xCE] = { &CPU::DEC, &CPU::ABS, 6 };
	ins_table[0xC6] = { &CPU::DEC, &CPU::DP, 5 };
	ins_table[0xDE] = { &CPU::DEC, &CPU::ABSX, 7 };
	ins_table[0xD6] = { &CPU::DEC, &CPU::DPX, 6 };

	ins_table[0xCA] = { &CPU::DEX, &CPU::IMP, 2 };
	ins_table[0x88] = { &CPU::DEY, &CPU::IMP, 2 };

	ins_table[0x49] = { &CPU::EOR, &CPU::IMM, 2 };
	ins_table[0x4D] = { &CPU::EOR, &CPU::ABS, 4 };
	ins_table[0x4F] = { &CPU::EOR, &CPU::ABSL, 5 };
	ins_table[0x45] = { &CPU::EOR, &CPU::DP, 3 };
	ins_table[0x52] = { &CPU::EOR, &CPU::DPI, 5 };
	ins_table[0x47] = { &CPU::EOR, &CPU::DPIL, 6 };
	ins_table[0x5D] = { &CPU::EOR, &CPU::ABSX, 4 };
	ins_table[0x5F] = { &CPU::EOR, &CPU::ABSLX, 5 };
	ins_table[0x59] = { &CPU::EOR, &CPU::ABSY, 4 };
	ins_table[0x55] = { &CPU::EOR, &CPU::DPX, 4 };
	ins_table[0x41] = { &CPU::EOR, &CPU::DPIX, 6 };
	ins_table[0x51] = { &CPU::EOR, &CPU::DPIY, 5 };
	ins_table[0x57] = { &CPU::EOR, &CPU::DPILY, 6 };
	ins_table[0x43] = { &CPU::EOR, &CPU::SR, 4 };
	ins_table[0x53] = { &CPU::EOR, &CPU::SRY, 7 };

	ins_table[0x1A] = { &CPU::INA, &CPU::ACC, 2 };
	ins_table[0xEE] = { &CPU::INC, &CPU::ABS, 6 };
	ins_table[0xE6] = { &CPU::INC, &CPU::DP, 5 };
	ins_table[0xFE] = { &CPU::INC, &CPU::ABSX, 7 };
	ins_table[0xF6] = { &CPU::INC, &CPU::DPX, 6 };

	ins_table[0xE8] = { &CPU::INX, &CPU::IMP, 2 };
	ins_table[0xC8] = { &CPU::INY, &CPU::IMP, 2 };

	ins_table[0x4C] = { &CPU::JMP, &CPU::ABS, 3 };
	ins_table[0x6C] = { &CPU::JMP, &CPU::ABSI, 5 };
	ins_table[0x7C] = { &CPU::JMP, &CPU::ABSIX, 6 };

	ins_table[0x5C] = { &CPU::JML, &CPU::ABSL, 4 };
	ins_table[0xDC] = { &CPU::JML, &CPU::ABSIL, 6 };

	ins_table[0x20] = { &CPU::JSR, &CPU::ABS, 6 };
	ins_table[0x22] = { &CPU::JSRL, &CPU::ABSL, 8 };
	ins_table[0xFC] = { &CPU::JSR, &CPU::ABSIX, 8 };

	ins_table[0xA9] = { &CPU::LDA, &CPU::IMM, 2 };
	ins_table[0xAD] = { &CPU::LDA, &CPU::ABS, 4 };
	ins_table[0xAF] = { &CPU::LDA, &CPU::ABSL, 5 };
	ins_table[0xA5] = { &CPU::LDA, &CPU::DP, 3 };
	ins_table[0xB2] = { &CPU::LDA, &CPU::DPI, 5 };
	ins_table[0xA7] = { &CPU::LDA, &CPU::DPIL, 6 };
	ins_table[0xBD] = { &CPU::LDA, &CPU::ABSX, 4 };
	ins_table[0xBF] = { &CPU::LDA, &CPU::ABSLX, 5 };
	ins_table[0xB9] = { &CPU::LDA, &CPU::ABSY, 4 };
	ins_table[0xB5] = { &CPU::LDA, &CPU::DPX, 4 };
	ins_table[0xA1] = { &CPU::LDA, &CPU::DPIX, 6 };
	ins_table[0xB1] = { &CPU::LDA, &CPU::DPIY, 5 };
	ins_table[0xB7] = { &CPU::LDA, &CPU::DPILY, 6 };
	ins_table[0xA3] = { &CPU::LDA, &CPU::SR, 4 };
	ins_table[0xB3] = { &CPU::LDA, &CPU::SRY, 7 };

	ins_table[0xA2] = { &CPU::LDX, &CPU::IMMX, 2 };
	ins_table[0xAE] = { &CPU::LDX, &CPU::ABS, 4 };
	ins_table[0xA6] = { &CPU::LDX, &CPU::DP, 3 };
	ins_table[0xBE] = { &CPU::LDX, &CPU::ABSY, 4 };
	ins_table[0xB6] = { &CPU::LDX, &CPU::DPY, 4 };

	ins_table[0xA0] = { &CPU::LDY, &CPU::IMMX, 2 };
	ins_table[0xAC] = { &CPU::LDY, &CPU::ABS, 4 };
	ins_table[0xA4] = { &CPU::LDY, &CPU::DP, 3 };
	ins_table[0xBC] = { &CPU::LDY, &CPU::ABSX, 4 };
	ins_table[0xB4] = { &CPU::LDY, &CPU::DPX, 4 };

	ins_table[0x4A] = { &CPU::LSRA, &CPU::ACC, 2 };
	ins_table[0x4E] = { &CPU::LSR, &CPU::ABS, 6 };
	ins_table[0x46] = { &CPU::LSR, &CPU::DP, 5 };
	ins_table[0x5E] = { &CPU::LSR, &CPU::ABSX, 7 };
	ins_table[0x56] = { &CPU::LSR, &CPU::DPX, 6 };

	ins_table[0x54] = { &CPU::MVN, &CPU::MOVE, 3 };
	ins_table[0x44] = { &CPU::MVP, &CPU::MOVE, 3 };

	ins_table[0xEA] = { &CPU::NOP, &CPU::IMP, 2 };

	ins_table[0x09] = { &CPU::ORA, &CPU::IMM, 2 };
	ins_table[0x0D] = { &CPU::ORA, &CPU::ABS, 4 };
	ins_table[0x0F] = { &CPU::ORA, &CPU::ABSL, 5 };
	ins_table[0x05] = { &CPU::ORA, &CPU::DP, 3 };
	ins_table[0x12] = { &CPU::ORA, &CPU::DPI, 5 };
	ins_table[0x07] = { &CPU::ORA, &CPU::DPIL, 6 };
	ins_table[0x1D] = { &CPU::ORA, &CPU::ABSX, 4 };
	ins_table[0x1F] = { &CPU::ORA, &CPU::ABSLX, 5 };
	ins_table[0x19] = { &CPU::ORA, &CPU::ABSY, 4 };
	ins_table[0x15] = { &CPU::ORA, &CPU::DPX, 4 };
	ins_table[0x01] = { &CPU::ORA, &CPU::DPIX, 6 };
	ins_table[0x11] = { &CPU::ORA, &CPU::DPIY, 5 };
	ins_table[0x17] = { &CPU::ORA, &CPU::DPILY, 6 };
	ins_table[0x03] = { &CPU::ORA, &CPU::SR, 4 };
	ins_table[0x13] = { &CPU::ORA, &CPU::SRY, 7 };

	ins_table[0xF4] = { &CPU::PEA, &CPU::IMM16, 5 };
	ins_table[0xD4] = { &CPU::PEI, &CPU::DPI, 6 };
	ins_table[0x62] = { &CPU::PER, &CPU::IMM16, 6 };

	ins_table[0x48] = { &CPU::PHA, &CPU::IMP, 3 };
	ins_table[0x08] = { &CPU::PHP, &CPU::IMP, 3 };
	ins_table[0xDA] = { &CPU::PHX, &CPU::IMP, 3 };
	ins_table[0x5A] = { &CPU::PHY, &CPU::IMP, 3 };
	ins_table[0x68] = { &CPU::PLA, &CPU::IMP, 4 };
	ins_table[0x28] = { &CPU::PLP, &CPU::IMP, 4 };
	ins_table[0xFA] = { &CPU::PLX, &CPU::IMP, 4 };
	ins_table[0x7A] = { &CPU::PLY, &CPU::IMP, 4 };
	ins_table[0x8B] = { &CPU::PHB, &CPU::IMP, 3 };
	ins_table[0x0B] = { &CPU::PHD, &CPU::IMP, 4 };
	ins_table[0x4B] = { &CPU::PHK, &CPU::IMP, 3 };
	ins_table[0xAB] = { &CPU::PLB, &CPU::IMP, 4 };
	ins_table[0x2B] = { &CPU::PLD, &CPU::IMP, 5 };

	ins_table[0xC2] = { &CPU::REP, &CPU::IMM8, 3 };

	ins_table[0x2A] = { &CPU::ROLA, &CPU::ACC, 2 };
	ins_table[0x2E] = { &CPU::ROL, &CPU::ABS, 6 };
	ins_table[0x26] = { &CPU::ROL, &CPU::DP, 5 };
	ins_table[0x3E] = { &CPU::ROL, &CPU::ABSX, 7 };
	ins_table[0x36] = { &CPU::ROL, &CPU::DPX, 6 };

	ins_table[0x6A] = { &CPU::RORA, &CPU::ACC, 2 };
	ins_table[0x6E] = { &CPU::ROR, &CPU::ABS, 6 };
	ins_table[0x66] = { &CPU::ROR, &CPU::DP, 5 };
	ins_table[0x7E] = { &CPU::ROR, &CPU::ABSX, 7 };
	ins_table[0x76] = { &CPU::ROR, &CPU::DPX, 6 };

	ins_table[0x40] = { &CPU::RTI, &CPU::IMP, 6 };
	ins_table[0x6B] = { &CPU::RTL, &CPU::IMP, 6 };
	ins_table[0x60] = { &CPU::RTS, &CPU::IMP, 6 };

	ins_table[0xE9] = { &CPU::SBC, &CPU::IMM, 2 };
	ins_table[0xED] = { &CPU::SBC, &CPU::ABS, 4 };
	ins_table[0xEF] = { &CPU::SBC, &CPU::ABSL, 5 };
	ins_table[0xE5] = { &CPU::SBC, &CPU::DP, 3 };
	ins_table[0xF2] = { &CPU::SBC, &CPU::DPI, 5 };
	ins_table[0xE7] = { &CPU::SBC, &CPU::DPIL, 6 };
	ins_table[0xFD] = { &CPU::SBC, &CPU::ABSX, 4 };
	ins_table[0xFF] = { &CPU::SBC, &CPU::ABSLX, 5 };
	ins_table[0xF9] = { &CPU::SBC, &CPU::ABSY, 4 };
	ins_table[0xF5] = { &CPU::SBC, &CPU::DPX, 4 };
	ins_table[0xE1] = { &CPU::SBC, &CPU::DPIX, 6 };
	ins_table[0xF1] = { &CPU::SBC, &CPU::DPIY, 5 };
	ins_table[0xF7] = { &CPU::SBC, &CPU::DPILY, 6 };
	ins_table[0xE3] = { &CPU::SBC, &CPU::SR, 4 };
	ins_table[0xF3] = { &CPU::SBC, &CPU::SRY, 7 };

	ins_table[0xE2] = { &CPU::SEP, &CPU::IMM8, 3 };

	ins_table[0x38] = { &CPU::SEC, &CPU::IMP, 2 };
	ins_table[0xF8] = { &CPU::SED, &CPU::IMP, 2 };
	ins_table[0x78] = { &CPU::SEI, &CPU::IMP, 2 };

	ins_table[0x8D] = { &CPU::STA, &CPU::ABS, 4 };
	ins_table[0x8F] = { &CPU::STA, &CPU::ABSL, 5 };
	ins_table[0x85] = { &CPU::STA, &CPU::DP, 3 };
	ins_table[0x92] = { &CPU::STA, &CPU::DPI, 5 };
	ins_table[0x87] = { &CPU::STA, &CPU::DPIL, 6 };
	ins_table[0x9D] = { &CPU::STA, &CPU::ABSX, 4 };
	ins_table[0x9F] = { &CPU::STA, &CPU::ABSLX, 5 };
	ins_table[0x99] = { &CPU::STA, &CPU::ABSY, 4 };
	ins_table[0x95] = { &CPU::STA, &CPU::DPX, 4 };
	ins_table[0x81] = { &CPU::STA, &CPU::DPIX, 6 };
	ins_table[0x91] = { &CPU::STA, &CPU::DPIY, 5 };
	ins_table[0x97] = { &CPU::STA, &CPU::DPILY, 6 };
	ins_table[0x83] = { &CPU::STA, &CPU::SR, 4 };
	ins_table[0x93] = { &CPU::STA, &CPU::SRY, 7 };

	ins_table[0xDB] = { &CPU::STP, &CPU::IMP, 3 };

	ins_table[0x8E] = { &CPU::STX, &CPU::ABS, 4 };
	ins_table[0x86] = { &CPU::STX, &CPU::DP, 3 };
	ins_table[0x96] = { &CPU::STX, &CPU::DPY, 4 };

	ins_table[0x8C] = { &CPU::STY, &CPU::ABS, 4 };
	ins_table[0x84] = { &CPU::STY, &CPU::DP, 3 };
	ins_table[0x94] = { &CPU::STY, &CPU::DPX, 4 };

	ins_table[0x9C] = { &CPU::STZ, &CPU::ABS, 4 };
	ins_table[0x64] = { &CPU::STZ, &CPU::DP, 3 };
	ins_table[0x9E] = { &CPU::STZ, &CPU::ABSX, 5 };
	ins_table[0x74] = { &CPU::STZ, &CPU::DPY, 4 };

	ins_table[0xAA] = { &CPU::TAX, &CPU::IMP, 2 };
	ins_table[0xA8] = { &CPU::TAY, &CPU::IMP, 2 };
	ins_table[0x8A] = { &CPU::TXA, &CPU::IMP, 2 };
	ins_table[0x98] = { &CPU::TYA, &CPU::IMP, 2 };
	ins_table[0xBA] = { &CPU::TSX, &CPU::IMP, 2 };
	ins_table[0x9A] = { &CPU::TXS, &CPU::IMP, 2 };
	ins_table[0x9B] = { &CPU::TXY, &CPU::IMP, 2 };
	ins_table[0xBB] = { &CPU::TYX, &CPU::IMP, 2 };

	ins_table[0x5B] = { &CPU::TCD, &CPU::IMP, 2 };
	ins_table[0x7B] = { &CPU::TDC, &CPU::IMP, 2 };
	ins_table[0x1B] = { &CPU::TCS, &CPU::IMP, 2 };
	ins_table[0x3B] = { &CPU::TSC, &CPU::IMP, 2 };

	ins_table[0x1C] = { &CPU::TRB, &CPU::ABS, 6 };
	ins_table[0x14] = { &CPU::TRB, &CPU::DP, 5 };

	ins_table[0x0C] = { &CPU::TSB, &CPU::ABS, 6 };
	ins_table[0x04] = { &CPU::TSB, &CPU::DP, 5 };

	ins_table[0xCB] = { &CPU::WAI, &CPU::IMP, 3 };

	ins_table[0x42] = { &CPU::WDM, &CPU::IMP, 2 };

	ins_table[0xEB] = { &CPU::XBA, &CPU::IMP, 3 };
	ins_table[0xFB] = { &CPU::XCE, &CPU::IMP, 2 };
}

void CPU::tick_components(unsigned mclock)
{
	snes->ppu.tick(mclock / 4);
}

void CPU::step()
{
#if 0
	if (PC == 0x8901) {
		printf("asdas");
	}
#endif

	opcode = read8(FULL_PC);
	PC++;
	
	(this->*ins_table[opcode].addr_mode)();
	(this->*ins_table[opcode].ins)();

	if (nmi_pending) nmi();

	if (irq_pending && !GET_I()) irq();
}

void CPU::nmi()
{
	push8(PBR);
	push16(PC);
	push8(status);

	SET_I(1);
	SET_D(0);
	PBR = 0;
	PC = read16(0xFFEA);

	nmi_pending = false;
	snes->bus.regs.nmitimen &= 0x7F;
	snes->ppu.set_nmi_enable(false);
}

void CPU::irq()
{
	push8(PBR);
	push16(PC);
	push8(status);

	SET_I(1);
	SET_D(0);
	PBR = 0;
	PC = read16(0xFFEE);

	irq_pending = false;
}

void CPU::reset()
{
	A = X = Y = 0;
	status = 0x34;
	SP = 0x1FFF;
	D = 0;
	E = true;
	DBR = 0;
	mclock = 0;
	nmi_pending = false;
	PC = read16(0xFFFC);
}

uint8_t CPU::read8(uint32_t addr)
{
	return snes->bus.read(addr);
}

void CPU::write8(uint32_t addr, uint8_t val)
{
	return snes->bus.write(addr, val);
}

uint16_t CPU::read16(uint32_t addr)
{
	uint32_t next_addr = (addr & 0xFF0000) | ((addr + 1) & 0xFFFF);
	return (read8(next_addr) << 8) | read8(addr);
}

void CPU::write16(uint32_t addr, uint16_t val)
{
	uint32_t next_addr = (addr & 0xFF0000) | ((addr + 1) & 0xFFFF);
	write8(next_addr, val >> 8);
	write8(addr, val & 0xFF);
}

void CPU::check_addr_mode()
{
	if (ins_table[opcode].addr_mode != &CPU::IMM && ins_table[opcode].addr_mode != &CPU::IMM8 &&
		ins_table[opcode].addr_mode != &CPU::IMM16 && ins_table[opcode].addr_mode != &CPU::IMMX)
		val = (GET_M()) ? read8(addr) : read16(addr);
}

void CPU::check_addr_modex()
{
	if (ins_table[opcode].addr_mode != &CPU::IMM && ins_table[opcode].addr_mode != &CPU::IMM8 &&
		ins_table[opcode].addr_mode != &CPU::IMM16 && ins_table[opcode].addr_mode != &CPU::IMMX)
		val = (GET_X()) ? read8(addr) : read16(addr);
}

void CPU::push8(uint8_t v)
{
	write8(SP, v);
	SP--;
}

void CPU::push16(uint16_t v)
{
	write8(SP, v >> 8);
	SP--;

	write8(SP, v & 0xFF);
	SP--;
}

uint8_t CPU::pop8()
{
	SP++;
	return read8(SP);
}

uint16_t CPU::pop16()
{
	uint16_t v = 0;

	SP++;
	v |= read8(SP);
	SP++;
	v |= read8(SP) << 8;

	return v;
}