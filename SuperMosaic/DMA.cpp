#include "DMA.h"
#include "SNES.h"

void DMA::dma_transfer(uint8_t idx)
{
	uint16_t n = channels[idx].byte_count;
	uint8_t bank = channels[idx].dma_bank;

	if (!n) n--;

	int inc = 0;
	switch ((channels[idx].dmaparam >> 3) & 3) {
		case 0: inc = 1; break;
		case 2: inc = -1; break;
	}

	switch (channels[idx].dmaparam & 0x7) {
		case 0:
			while (n) {
				if (channels[idx].dmaparam & 0x80) {
					uint8_t byte = snes->bus.read(channels[idx].addressB);
					snes->bus.write((bank << 16) | channels[idx].addressA, byte);

					channels[idx].addressA += inc;
					n--;
				}
				else {
					uint8_t byte = snes->bus.read((bank << 16) | channels[idx].addressA);
					snes->bus.write(channels[idx].addressB, byte);

					channels[idx].addressA += inc;
					n--;
				}
			}
			break;

		case 1:
			while (n) {
				if (channels[idx].dmaparam & 0x80) {
					for (int i = 0; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read(channels[idx].addressB + i);
						snes->bus.write((bank << 16) | channels[idx].addressA, byte);

						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					for (int i = 0; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read((bank << 16) | channels[idx].addressA);
						snes->bus.write(channels[idx].addressB + i, byte);

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
						uint8_t byte = snes->bus.read(channels[idx].addressB);
						snes->bus.write((bank << 16) | channels[idx].addressA, byte);

						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					for (int i = 0; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read((bank << 16) | channels[idx].addressA);
						snes->bus.write(channels[idx].addressB, byte);

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
						uint8_t byte = snes->bus.read(channels[idx].addressB);
						snes->bus.write((bank << 16) | channels[idx].addressA, byte);

						channels[idx].addressA += inc;
						n--;
					}

					for (; i < 4 && n; i++) {
						uint8_t byte = snes->bus.read(channels[idx].addressB + 1);
						snes->bus.write((bank << 16) | channels[idx].addressA, byte);

						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					int i = 0;

					for (; i < 2 && n; i++) {
						uint8_t byte = snes->bus.read((bank << 16) | channels[idx].addressA);
						snes->bus.write(channels[idx].addressB, byte);

						channels[idx].addressA += inc;
						n--;
					}

					for (; i < 4 && n; i++) {
						uint8_t byte = snes->bus.read((bank << 16) | channels[idx].addressA);
						snes->bus.write(channels[idx].addressB + 1, byte);

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
						uint8_t byte = snes->bus.read(channels[idx].addressB + i);
						snes->bus.write((bank << 16) | channels[idx].addressA, byte);

						channels[idx].addressA += inc;
						n--;
					}
				}
				else {
					for (int i = 0; i < 4 && n; i++) {
						uint8_t byte = snes->bus.read((bank << 16) | channels[idx].addressA);
						snes->bus.write(channels[idx].addressB + i, byte);

						channels[idx].addressA += inc;
						n--;
					}
				}
			}
			break;
	}

	channels[idx].byte_count = n;
}

DMA::DMA(SNES* snes) : snes(snes)
{
	memset(channels, 0xFF, sizeof(channels));
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
			return tmp.addressA >> 8 & 0xFF;

		case 0x4:
			return tmp.dma_bank;

		case 0x5:
			return tmp.byte_count & 0xFF;

		case 0x6:
			return (tmp.byte_count >> 8) & 0xFF;

		case 0x7:
			return tmp.hdma_bank;

		case 0x8:
			return tmp.hdma_cur_addr & 0xFF;

		case 0x9:
			return (tmp.hdma_cur_addr >> 8) & 0xFF;

		case 0xA:
			return tmp.hdma_rl;

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
			tmp.addressA = (tmp.addressA & 0xFFFF00) | val;
			break;

		case 0x3:
			tmp.addressA = (tmp.addressA & 0xFF00FF) | (val << 8);
			break;

		case 0x4:
			tmp.dma_bank = val;
			break;

		case 0x5:
			tmp.byte_count = (tmp.byte_count & 0xFF00) | val;
			break;

		case 0x6:
			tmp.byte_count = (tmp.byte_count & 0x00FF) | (val << 8);
			break;

		case 0x7:
			tmp.hdma_bank = val;
			break;

		case 0x8:
			tmp.hdma_cur_addr = (tmp.hdma_cur_addr & 0xFF00) | (val << 8);
			break;

		case 0x9:
			tmp.hdma_cur_addr = (tmp.hdma_cur_addr & 0x00FF) | val;
			break;

		case 0xA:
			tmp.hdma_rl = val;
			break;

		case 0xB:
			tmp.unused = val;
			break;
	}
}
