#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <fstream>
#include <iostream>

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

#define FULL_PC ((PBR << 16) | PC)

class SNES;

class CPU
{
	uint16_t A;
	uint16_t X;
	uint16_t Y;

	uint16_t SP;
	uint16_t PC;
	uint8_t PBR;
	uint16_t D;
	uint8_t DBR;

	uint8_t status;
	bool E;

	uint16_t val;
	uint32_t addr;
	uint8_t opcode;
	unsigned int mclock;

	struct InsEntry {
		uint8_t (CPU::*ins)();
		void (CPU::*addr_mode)();
		unsigned cycles;
	};

	std::array<InsEntry, 256> ins_table;

	SNES* snes;

	void check_addr_mode();
	void check_addr_modex();

	void push8(uint8_t v);
	void push16(uint16_t v);

	uint8_t pop8();
	uint16_t pop16();

	// Addressing Modes
	void IMP();
	void IMM();
	void IMM8();
	void IMM16();
	void IMMX();
	void ACC();
	
	void DP();
	void DPI();
	void DPIL();
	void DPX();
	void DPY();
	void DPIX();
	void DPIY();
	void DPILY();

	void REL();
	void RELL();

	void ABS();
	void ABSL();
	void ABSX();
	void ABSLX();
	void ABSY();
	void ABSI();
	void ABSIL();
	void ABSIX();

	void SR();
	void SRY();

	void MOVE();

	// Instructions
	uint8_t ADC();
	uint8_t AND();
	uint8_t ASL();
	uint8_t ASLA();

	uint8_t BCC();
	uint8_t BCS();
	uint8_t BEQ();
	uint8_t BIT();
	uint8_t BIT_IMM();
	uint8_t BMI();
	uint8_t BNE();
	uint8_t BPL();
	uint8_t BRA();
	uint8_t BRL();
	uint8_t BRK();
	uint8_t BVC();
	uint8_t BVS();

	uint8_t CLC();
	uint8_t CLD();
	uint8_t CLI();
	uint8_t CLV();
	uint8_t CMP();
	uint8_t COP();
	uint8_t CPX();
	uint8_t CPY();

	uint8_t DEC();
	uint8_t DEA();
	uint8_t DEX();
	uint8_t DEY();

	uint8_t EOR();

	uint8_t INC();
	uint8_t INA();
	uint8_t INX();
	uint8_t INY();

	uint8_t JMP();
	uint8_t JML();
	uint8_t JSR();
	uint8_t JSRL();

	uint8_t LDA();
	uint8_t LDX();
	uint8_t LDY();
	uint8_t LSR();
	uint8_t LSRA();

	uint8_t MVN();
	uint8_t MVP();

	uint8_t NOP();

	uint8_t ORA();

	uint8_t PEA();
	uint8_t PEI();
	uint8_t PER();
	uint8_t PHA();
	uint8_t PHB();
	uint8_t PHD();
	uint8_t PHK();
	uint8_t PHP();
	uint8_t PHX();
	uint8_t PHY();
	uint8_t PLA();
	uint8_t PLB();
	uint8_t PLD();
	uint8_t PLP();
	uint8_t PLX();
	uint8_t PLY();

	uint8_t REP();
	uint8_t ROL();
	uint8_t ROLA();
	uint8_t ROR();
	uint8_t RORA();
	uint8_t RTI();
	uint8_t RTL();
	uint8_t RTS();

	uint8_t SBC();
	uint8_t SEP();
	uint8_t STP();
	uint8_t SEC();
	uint8_t SED();
	uint8_t SEI();
	uint8_t STA();
	uint8_t STX();
	uint8_t STY();
	uint8_t STZ();

	uint8_t TAX();
	uint8_t TAY();
	uint8_t TCD();
	uint8_t TDC();
	uint8_t TSC();
	uint8_t TCS();
	uint8_t TSX();
	uint8_t TXA();
	uint8_t TXS();
	uint8_t TXY();
	uint8_t TYA();
	uint8_t TYX();

	uint8_t TRB();
	uint8_t TSB();

	uint8_t WAI();
	uint8_t WDM();

	uint8_t XBA();
	uint8_t XCE();

public:
	bool nmi_pending;

	CPU(SNES* snes);

	void tick_components(unsigned mclock);

	void step();

	void nmi();
	void reset();

	uint8_t read8(uint32_t addr);
	void write8(uint32_t addr, uint8_t val);

	uint16_t read16(uint32_t addr);
	void write16(uint32_t addr, uint16_t val);
};

