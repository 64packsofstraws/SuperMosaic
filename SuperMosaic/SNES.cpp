#include "SNES.h"

bool SNES::is_title(std::vector<uint8_t>& rom, uint16_t addr)
{
	for (int i = addr; i < addr + 21 && rom[i] != 0; i++) {
		if (!isalnum(rom[i]) && !isspace(rom[i])) return false;
	}

	return true;
}

SNES::SNES(bool mouse_enabled) : cpu(this), bus(this), ppu(this), dma(this)
{
	SDL_Init(SDL_INIT_VIDEO);

	memset(&header, 0, sizeof(header));

	if (mouse_enabled) {
		ctrlr = std::make_shared<SNESMouse>(this);
	}
	else {
		ctrlr = std::make_shared<SNESController>(this);
	}
}

void SNES::load_file(const char* filename)
{
	std::ifstream f(filename, std::ios::binary);

	if (!f) {
		std::cerr << "Error: ROM not found\n";
		exit(1);
	}

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
	size_t ram_size = !header.ram_size ? 0 : (1 << header.ram_size) * 1024;

	if (rom_size < rom_data.size()) rom_size = rom_data.size();

	std::vector<uint8_t> rom(rom_size, 0);
	std::copy_n(rom_data.begin(), rom.size(), rom.begin());

	std::vector<uint8_t> ram(ram_size, 0);

	switch (header.map_mode & 0xF) {
		case 0: cart = std::make_unique<LoROM>(rom, ram, header.map_mode & 0x10); break;
		case 1: cart = std::make_unique<HiROM>(rom, ram, header.map_mode & 0x10); break;
		
		default:
			printf("Cart type not supported\n");
			exit(1);
	}

	cpu.reset();
}

void SNES::run()
{
	bool running = true;
	SDL_Event e;

	while (running) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;

				case SDL_EVENT_MOUSE_MOTION:
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				case SDL_EVENT_KEY_DOWN:
					ctrlr->handle_ctrl_in(e);
					break;

				case SDL_EVENT_MOUSE_BUTTON_UP:
				case SDL_EVENT_KEY_UP:
					ctrlr->handle_ctrl_out(e);
					break;
			}
		}

		while (!ppu.frame_ready)
			cpu.step();

		ppu.render();
	}
}
