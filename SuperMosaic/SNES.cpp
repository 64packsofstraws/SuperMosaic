#include "SNES.h"

bool SNES::is_title(std::vector<uint8_t>& rom, uint16_t addr)
{
	for (int i = addr; i < addr + 21 && rom[i] != 0; i++) {
		if (rom[i] < 32) return false;
	}

	return true;
}

SNES::SNES() : cpu(this), bus(this), ppu(this), dma(this)
{
	memset(&header, 0, sizeof(header));
}

void SNES::load_file(const char* filename)
{
	std::ifstream f(filename, std::ios::binary);
	std::filesystem::path p(filename);

	size_t file_size = std::filesystem::file_size(p);
	std::vector<uint8_t> rom_data(file_size);
	
	f.read(reinterpret_cast<char*>(rom_data.data()), file_size);
	f.close();

	unsigned extra_header = file_size % 1024;

	if (extra_header != 0x200 && extra_header != 0) {
		std::cerr << "Error with loading file\n";
		exit(1);
	}
	
	std::vector<uint16_t> header_addrs = { 0x7FC0, 0xFFC0 };

	uint32_t header_addr = 0;
	for (auto i : header_addrs) {
		unsigned off = extra_header + i;

		if (off + 0x1F > file_size) continue;

		uint16_t hdr_checksum = (rom_data[off + 0x1F] << 8) | rom_data[off + 0x1E];
		uint16_t comp_checksum  = (rom_data[off + 0x1D] << 8) | rom_data[off + 0x1C];

		if (comp_checksum == (hdr_checksum ^ 0xFFFF) || is_title(rom_data, off)) {
			header_addr = off - extra_header;
			break;
		}
	}

	if (!header_addr) {
		std::cerr << "Error: Couldn't find header\n";
		exit(1);
	}
	
	if (extra_header == 0x200)
		rom_data.erase(rom_data.begin(), rom_data.begin() + 0x200);

	auto header_it = rom_data.begin() + header_addr;
	std::copy(header_it, header_it + 32, reinterpret_cast<uint8_t*>(&header));

	size_t rom_size = (1 << header.rom_size) * 1024;
	size_t ram_size = (1 << header.ram_size) * 1024;

	if (rom_size < rom_data.size()) rom_size = rom_data.size();

	std::vector<uint8_t> rom(rom_size, 0);
	std::copy_n(rom_data.begin(), rom.size(), rom.begin());

	std::vector<uint8_t> ram(ram_size, 0);

	switch (header.map_mode & 0xF) {
		case 0: cart = std::make_unique<LoROM>(rom, ram, header.map_mode & 0x10); break;
	}

	cpu.reset();
}

void SNES::run()
{
	bool running = true;
	SDL_Event e;
	Uint64 tick = SDL_GetTicks();

	while (running) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;

				case SDL_EVENT_KEY_DOWN:
					bus.handle_joyp_in(e.key.key);
					break;

				case SDL_EVENT_KEY_UP:
					bus.handle_joyp_out(e.key.key);
			}
		}

		cpu.step();

		if (ppu.frame_ready) {
			ppu.render();
		
			tick = SDL_GetTicks() - tick;

			if (tick < 16) {
				SDL_Delay(16 - tick);
				tick = SDL_GetTicks();
			}
		}
	}
}
