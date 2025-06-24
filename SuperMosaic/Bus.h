#pragma once
#include <cstdint>
#include <vector>
#include <SDL3/SDL_keycode.h>

class SNES;

class Bus
{
	SNES* snes;
	std::vector<uint8_t> wram;

	struct Regs {
		uint8_t apuio[4];
		uint8_t wmdata;
		uint8_t wmaddl;
		uint8_t wmaddm;
		uint8_t wmaddh;
		uint8_t joyout;
		uint8_t joyser0;
		uint8_t joyser1;
		uint8_t nmitimen;
		uint8_t wrio;
		uint8_t wrmpya;
		uint8_t wrmpyb;
		uint8_t wrdivl;
		uint8_t wrdivh;
		uint8_t wrdivb;
		uint16_t htime;
		uint16_t vtime;
		uint8_t mdmaen;
		uint8_t hdmaen;
		uint8_t memsel;
		uint8_t rdnmi;
		uint8_t timeup;
		uint8_t hvbjoy;
		uint8_t rdio;
		uint8_t rddivl;
		uint8_t rddivh;
		uint8_t rdmpyl;
		uint8_t rdmpyh;
		uint16_t joy1;
	};

	uint32_t wram_addr;

	uint8_t mdr;

	uint8_t read_regs(uint16_t addr);
	void write_regs(uint16_t addr, uint8_t val);

public:
	Regs regs;
	
	Bus(SNES* snes);

	void handle_joyp_in(SDL_Keycode k);
	void handle_joyp_out(SDL_Keycode k);

	uint8_t read(uint32_t addr);
	void write(uint32_t addr, uint8_t val);
};