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
		uint8_t bank;
		uint16_t byte_count;
		uint8_t hdma_ind_bank;
		uint16_t hdma_cur_addr;
		uint8_t hdma_lc;
		uint8_t unused;
		bool hdma_enabled;
		bool hdma_terminated;
	} channels[8];

	int scanlines_waited;
	uint8_t org_lc;
	bool write_once;

	SNES* snes;

public:
	uint8_t hdmaen;

	DMA(SNES* snes);

	uint8_t get_hdma_lc(uint8_t idx);

	void dma_transfer(uint8_t idx);
	void hdma_transfer(uint8_t idx);

	void start_hdma_transfer(uint8_t idx);

	void hdma_init();
	void hdma_reset();

	bool hdma_is_enabled(uint8_t idx) const;
	bool hdma_is_terminated(uint8_t idx) const;

	uint8_t read_reg(uint16_t addr);
	void write_reg(uint16_t addr, uint8_t val);
};

