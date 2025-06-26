#include "DMA.h"
#include "SNES.h"

void DMA::dma_transfer(uint8_t idx)
{
	uint16_t n = channels[idx].das;
	uint8_t bank = channels[idx].bank;

	if (!n) n--;

	int inc = 0;
	switch ((channels[idx].dmaparam >> 3) & 0x3) {
		case 0: inc = 1; break;
		case 2: inc = -1; break;
	}

	switch (channels[idx].dmaparam & 0x7) {
		case 0:
			while (n) {
				if (channels[idx].dmaparam & 0x80) {
					uint8_t byte = snes->bus.read_noticks(channels[idx].addressB);
					snes->bus.write_noticks((bank << 16) | channels[idx].addressA, byte);

					snes->cpu.tick_components(8);
					channels[idx].addressA += inc;
					n--;
				}
				else {
					uint8_t byte = snes->bus.read_noticks((bank << 16) | channels[idx].addressA);
					snes->bus.write_noticks(channels[idx].addressB, byte);

					snes->cpu.tick_components(8);
					channels[idx].addressA += inc;
					n--;
				}
			}
			break;

		case 1:
			while (n) {
				if (channels[idx].dmaparam & 0x80) {
					for (int i = 0; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read_noticks(channels[idx].addressB + i);
						snes->bus.write_noticks((bank << 16) | channels[idx].addressA, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					for (int i = 0; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read_noticks((bank << 16) | channels[idx].addressA);
						snes->bus.write_noticks(channels[idx].addressB + i, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
			}
			break;

		case 2:
			while (n) {
				if (channels[idx].dmaparam & 0x80) {
					for (int i = 0; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read_noticks(channels[idx].addressB);
						snes->bus.write_noticks((bank << 16) | channels[idx].addressA, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					for (int i = 0; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read_noticks((bank << 16) | channels[idx].addressA);
						snes->bus.write_noticks(channels[idx].addressB, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
			}
			break;

		case 3:
			while (n) {
				if (channels[idx].dmaparam & 0x80) {
					int i = 0;

					for (; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read_noticks(channels[idx].addressB);
						snes->bus.write_noticks((bank << 16) | channels[idx].addressA, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}

					for (; i < 4 && n; i++) {
						uint8_t byte = snes->bus.read_noticks(channels[idx].addressB + 1);
						snes->bus.write_noticks((bank << 16) | channels[idx].addressA, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					int i = 0;

					for (; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read_noticks((bank << 16) | channels[idx].addressA);
						snes->bus.write_noticks(channels[idx].addressB, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}

					for (; i < 4 && n; i++) {
						uint8_t byte = snes->bus.read_noticks((bank << 16) | channels[idx].addressA);
						snes->bus.write_noticks(channels[idx].addressB + 1, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
			}
			break;

		case 4:
			while (n) {
				if (channels[idx].dmaparam & 0x80) {
					for (int i = 0; i < 4 && n; i++) {
						uint8_t byte = snes->bus.read_noticks(channels[idx].addressB + i);
						snes->bus.write_noticks((bank << 16) | channels[idx].addressA, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					for (int i = 0; i < 4 && n; i++) {
						uint8_t byte = snes->bus.read_noticks((bank << 16) | channels[idx].addressA);
						snes->bus.write_noticks(channels[idx].addressB + i, byte);

						snes->cpu.tick_components(8);
						channels[idx].addressA += inc;
						n--;
					}
				}
			}
			break;
	}

	channels[idx].das = n;
}

void DMA::hdma_transfer(uint8_t idx)
{
	uint8_t bank = channels[idx].bank;
	uint32_t src, dest;

	if (channels[idx].dmaparam & 0x80) {
		src = channels[idx].addressB;
		dest = (bank << 16) | (channels[idx].hdma_cur_addr + 1);
	}
	else {
		dest = channels[idx].addressB;
		src = (bank << 16) | (channels[idx].hdma_cur_addr + 1);
	}

	switch (channels[idx].dmaparam & 0x7) {
		case 0: {
			uint8_t byte = snes->bus.read_noticks(src);
			snes->bus.write_noticks(dest, byte);

			snes->cpu.tick_components(8);
		}
			break;

		case 1: {
			uint8_t byte = snes->bus.read_noticks(src);
			snes->bus.write_noticks(dest, byte);
			snes->cpu.tick_components(8);

			byte = snes->bus.read_noticks(src + 1);
			snes->bus.write_noticks(dest + 1, byte);
			snes->cpu.tick_components(8);
		}
			break;

		case 2: {
			uint8_t byte = snes->bus.read_noticks(src);
			snes->bus.write_noticks(dest, byte);
			snes->cpu.tick_components(8);

			byte = snes->bus.read_noticks(src + 1);
			snes->bus.write_noticks(dest, byte);
			snes->cpu.tick_components(8);
		}
			break;

		case 3: {
			for (int i = 0; i < 4; i++) {
				uint8_t byte = snes->bus.read_noticks(src + i);
				snes->bus.write_noticks(dest + (i / 2), byte);
				snes->cpu.tick_components(8);
			}
		}
			break;

		case 4: {
			for (int i = 0; i < 4; i++) {
				uint8_t byte = snes->bus.read_noticks(src + i);
				snes->bus.write_noticks(dest + i, byte);
				snes->cpu.tick_components(8);
			}
		}
			break;
	}
}

void DMA::ind_hdma_transfer(uint8_t idx)
{
	uint32_t src, dest;

	if (channels[idx].dmaparam & 0x80) {
		src = channels[idx].addressB;
		dest = (channels[idx].bank << 16) | (channels[idx].hdma_cur_addr + 1);
	}
	else {
		dest = channels[idx].addressB;
		src = (channels[idx].bank << 16) | (channels[idx].hdma_cur_addr + 1);
	}

	uint8_t bank = channels[idx].hdma_ind_bank;

	switch (channels[idx].dmaparam & 0x7) {
		case 0: {
			uint8_t hi = snes->bus.read_noticks(src + 1);
			uint8_t lo = snes->bus.read_noticks(src);
			uint32_t full_addr = (bank << 16) | (hi << 8) | lo;
			channels[idx].das = full_addr & 0xFFFF;

			uint8_t byte = snes->bus.read_noticks(full_addr);
			snes->bus.write_noticks(dest, byte);
			snes->cpu.tick_components(8);
		}
			break;

		case 1: {
			for (int i = 0; i < 2; i++) {
				uint8_t hi = snes->bus.read_noticks(src + 1);
				uint8_t lo = snes->bus.read_noticks(src);
				uint32_t full_addr = (bank << 16) | ((hi << 8) | lo + i);
				channels[idx].das = full_addr & 0xFFFF;

				uint8_t byte = snes->bus.read_noticks(full_addr);
				snes->bus.write_noticks(dest + i, byte);
				snes->cpu.tick_components(8);
			}
		}
			  break;

		case 2: {
			uint8_t hi = snes->bus.read_noticks(src + 1);
			uint8_t lo = snes->bus.read_noticks(src);
			uint32_t full_addr = (bank << 16) | (hi << 8) | lo;

			uint8_t byte = snes->bus.read_noticks(full_addr);
			snes->bus.write_noticks(dest, byte);

			snes->cpu.tick_components(8);

			hi = snes->bus.read_noticks(src + 1);
			lo = snes->bus.read_noticks(src);
			full_addr = (bank << 16) | ((hi << 8) | lo + 1);
			channels[idx].das = full_addr & 0xFFFF;

			byte = snes->bus.read_noticks(full_addr);
			snes->bus.write_noticks(dest + 1, byte);

			snes->cpu.tick_components(8);
		}
			  break;

		case 3: {
			for (int i = 0; i < 4; i++) {
				uint8_t hi = snes->bus.read_noticks(src + 1);
				uint8_t lo = snes->bus.read_noticks(src);
				uint32_t full_addr = (bank << 16) | ((hi << 8) | lo + i);
				channels[idx].das = full_addr & 0xFFFF;

				uint8_t byte = snes->bus.read_noticks(full_addr);
				snes->bus.write_noticks(dest + (i / 2), byte);

				snes->cpu.tick_components(8);
			}
		}
			  break;

		case 4: {
			for (int i = 0; i < 4; i++) {
				uint8_t hi = snes->bus.read_noticks(src + 1);
				uint8_t lo = snes->bus.read_noticks(src);
				uint32_t full_addr = (bank << 16) | ((hi << 8) | lo + i);
				channels[idx].das = full_addr & 0xFFFF;

				uint8_t byte = snes->bus.read_noticks(full_addr);
				snes->bus.write_noticks(dest + i, byte);

				snes->cpu.tick_components(8);
			}
		}
			  break;
	}
}

void DMA::start_hdma_transfer(uint8_t idx)
{
	int n = 0;

	switch (channels[idx].dmaparam & 0x7) {
		case 0: n = 1; break;
		case 1: case 2: n = 2; break;
		case 3: case 4: n = 4; break;
	}

	if (!(channels[idx].hdma_lc & 0x7F)) {
		if (!(channels[idx].dmaparam & 0x40)) channels[idx].hdma_cur_addr += n + 1;
		else channels[idx].hdma_cur_addr += 3;

		channels[idx].hdma_lc = snes->bus.read_noticks((channels[idx].bank << 16) | channels[idx].hdma_cur_addr);

		if (!channels[idx].hdma_lc) {
			channels[idx].hdma_terminated = true;
			return;
		}

		if (!(channels[idx].hdma_lc & 0x80)) {
			write_once = true;
		}
	}

	bool repeat_mode = channels[idx].hdma_lc & 0x80;

	if (repeat_mode) {
		if (!(channels[idx].dmaparam & 0x40)) hdma_transfer(idx);
		else ind_hdma_transfer(idx);
	}
	else if (!repeat_mode && write_once) {
		if (!(channels[idx].dmaparam & 0x40)) hdma_transfer(idx);
		else ind_hdma_transfer(idx);
		write_once = false;
	}

	channels[idx].hdma_lc--;
}

void DMA::hdma_init()
{
	for (int i = 0; i < 8; i++) {
		if (hdmaen & (1 << i)) {
			channels[i].hdma_cur_addr = channels[i].addressA;
			channels[i].hdma_lc = snes->bus.read_noticks((channels[i].bank << 16) | channels[i].hdma_cur_addr);
			
			if (!(channels[i].hdma_lc & 0x80)) write_once = true;

			org_lc = channels[i].hdma_lc;
			channels[i].hdma_enabled = true;
			channels[i].hdma_terminated = false;
		}
		else {
			channels[i].hdma_enabled = false;
			channels[i].hdma_terminated = true;
		}
	}
}

void DMA::hdma_reset()
{
	for (int i = 0; i < 8; i++) {
		if (channels[i].hdma_enabled) {
			channels[i].hdma_cur_addr = channels[i].addressA;
			channels[i].hdma_lc = snes->bus.read_noticks((channels[i].bank << 16) | channels[i].hdma_cur_addr);

			if (!(channels[i].hdma_lc & 0x80)) write_once = true;
			channels[i].hdma_terminated = false;
		}
	}
}

bool DMA::hdma_is_enabled(uint8_t idx) const
{
	return channels[idx].hdma_enabled;
}

bool DMA::hdma_is_terminated(uint8_t idx) const
{
	return channels[idx].hdma_terminated;
}

DMA::DMA(SNES* snes) : snes(snes)
{
	memset(channels, 0xFF, sizeof(channels));
	hdmaen = 0;
	scanlines_waited = 0;
	org_lc = 0;
	write_once = false;
}

uint8_t DMA::get_hdma_lc(uint8_t idx)
{
	return channels[idx].hdma_lc;
}

uint8_t DMA::read_reg(uint16_t addr)
{
	Entry& tmp = channels[(addr >> 4) & 0x7];

	switch (addr & 0xF) {
		case 0x0:
			return tmp.dmaparam;

		case 0x1:
			return tmp.addressB & 0xFF;

		case 0x2:
			return tmp.addressA & 0xFF;

		case 0x3:
			return (tmp.addressA >> 8) & 0xFF;

		case 0x4:
			return tmp.bank;

		case 0x5:
			return tmp.das & 0xFF;

		case 0x6:
			return (tmp.das >> 8) & 0xFF;

		case 0x7:
			return tmp.hdma_ind_bank;

		case 0x8:
			return tmp.hdma_cur_addr & 0xFF;

		case 0x9:
			return (tmp.hdma_cur_addr >> 8) & 0xFF;

		case 0xA:
			return tmp.hdma_lc;

		case 0xB:
			return tmp.unused;
	}
	return 0;
}

void DMA::write_reg(uint16_t addr, uint8_t val)
{
	Entry& tmp = channels[(addr >> 4) & 0x7];

	switch (addr & 0xF) {
		case 0x0:
			tmp.dmaparam = val;
			break;

		case 0x1:
			tmp.addressB = 0x2100 | val;
			break;

		case 0x2:
			tmp.addressA = (tmp.addressA & 0xFF00) | val;
			break;

		case 0x3:
			tmp.addressA = (tmp.addressA & 0x00FF) | (val << 8);
			break;

		case 0x4:
			tmp.bank = val;
			break;

		case 0x5:
			tmp.das = (tmp.das & 0xFF00) | val;
			break;

		case 0x6:
			tmp.das = (tmp.das & 0x00FF) | (val << 8);
			break;

		case 0x7:
			tmp.hdma_ind_bank = val;
			break;

		case 0x8:
			tmp.hdma_cur_addr = (tmp.hdma_cur_addr & 0xFF00) | (val << 8);
			break;

		case 0x9:
			tmp.hdma_cur_addr = (tmp.hdma_cur_addr & 0x00FF) | val;
			break;

		case 0xA:
			tmp.hdma_lc = val;
			break;

		case 0xB:
			tmp.unused = val;
			break;
	}
}
