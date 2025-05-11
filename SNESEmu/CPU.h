#pragma once
#include <cstdint>
#include <vector>
#include <array>

class CPU
{
	uint16_t A;
	uint16_t X;
	uint16_t Y;

	uint16_t SP;
	uint32_t PC : 24;
	uint16_t D;
	uint8_t DBR;

	uint8_t status;
	bool E : 1;

	std::vector<uint8_t> memory;

	uint16_t val;
	uint32_t addr;
	uint8_t opcode;

	void check_addr_mode();

	void push8(uint8_t v);
	void push16(uint16_t v);

	uint8_t pop8();
	uint16_t pop16();

	// Addressing Modes
	uint8_t IMP();
	uint8_t IMM();
	uint8_t ACC();
	
	uint8_t DP();
	uint8_t DPI();
	uint8_t DPIL();
	uint8_t DPX();
	uint8_t DPY();

	uint8_t REL();

	uint8_t ABS();
	uint8_t ABSL();
	uint8_t ABSX();
	uint8_t ABSLX();
	uint8_t ABSY();

	// Instructions
	uint8_t ADC();
	uint8_t AND();
	uint8_t ASL();

	uint8_t BCC();
	uint8_t BCS();
	uint8_t BEQ();
	uint8_t BIT();
	uint8_t BMI();
	uint8_t BNE();
	uint8_t BPL();
	uint8_t BRK();
	uint8_t BVC();
	uint8_t BVS();

	uint8_t CLC();
	uint8_t CLD();
	uint8_t CLI();
	uint8_t CLV();
	uint8_t CMP();
	uint8_t CPX();
	uint8_t CPY();

	uint8_t DEC();
	uint8_t DEX();
	uint8_t DEY();

	uint8_t EOR();

	uint8_t INC();
	uint8_t INX();
	uint8_t INY();

	uint8_t JMP();
	uint8_t JSR();

	uint8_t LDA();
	uint8_t LDX();
	uint8_t LDY();
	uint8_t LSR();

	uint8_t NOP();

	uint8_t ORA();

	uint8_t PHA();
	uint8_t PHP();
	uint8_t PLA();
	uint8_t PLP();
	uint8_t ROL();
	uint8_t ROR();
	uint8_t RTI();
	uint8_t RTS();

	uint8_t SBC();
	uint8_t SEC();
	uint8_t SED();
	uint8_t SEI();
	uint8_t STA();
	uint8_t STX();
	uint8_t STY();

	uint8_t TAX();
	uint8_t TAY();
	uint8_t TSX();
	uint8_t TXA();
	uint8_t TXS();
	uint8_t TYA();

public:
	CPU();

	uint8_t step();

	void nmi();
	void reset();

	uint8_t cpu_read(uint16_t addr);
	void cpu_write(uint16_t addr, uint8_t val);
};

