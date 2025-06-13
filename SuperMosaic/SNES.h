#pragma once
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <memory>
#include <cmath>
#include "CPU.h"
#include "Cartridge.h"
#include "Bus.h"
#include "PPU.h"
#include "DMA.h"

#define PRINT(expr) #expr

#define SCALE 2

class SNES
{
	struct Header {
		uint8_t title[21];
		uint8_t map_mode;
		uint8_t chipset;
		uint8_t rom_size;
		uint8_t ram_size;
		uint8_t country;
		uint8_t dev_id;
		uint8_t rom_version;
		uint16_t comp_checksum;
		uint16_t checksum;
	} header;

	bool is_title(std::vector<uint8_t>& rom, uint16_t addr);
public:
	std::unique_ptr<Cartridge> cart;
	PPU ppu;
	CPU cpu;
	Bus bus;
	DMA dma;

	SNES();

	void load_file(const char* filename);
	void run();
};

