#pragma once
#include <cstdint>
#include <SDL3/SDL_keycode.h>

class SNES;

class Joypad
{
	uint16_t joypad;

	SNES* snes;
public:
	uint16_t shift_reg;

	Joypad(SNES* snes);

	void handle_joyp_in(SDL_Keycode k);
	void handle_joyp_out(SDL_Keycode k);

	void update_autoread();
	void update_shiftreg();

	uint8_t get_serinput();
};

