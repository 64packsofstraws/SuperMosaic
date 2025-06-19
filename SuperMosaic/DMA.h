#pragma once
#include <cstdint>
#include <cstring>

class SNES;

class DMA
{
	struct Entry {
		uint8_t dmaparam;
		uint16_t addressB;
		uint16_t addressA;
		uint8_t dma_bank;
		uint16_t byte_count;
		uint8_t hdma_bank;
		uint16_t hdma_cur_addr;
		uint8_t hdma_rl;
		uint8_t unused;
	} channels[8];

	SNES* snes;

public:
	DMA(SNES* snes);

	void dma_transfer(uint8_t idx);
	
	uint8_t read_reg(uint16_t addr);
	void write_reg(uint16_t addr, uint8_t val);
};

