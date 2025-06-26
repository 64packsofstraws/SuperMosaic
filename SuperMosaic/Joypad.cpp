#include "Joypad.h"
#include "SNES.h"

Joypad::Joypad(SNES* snes) : snes(snes)
{
	joypad = 0;
}

void Joypad::handle_joyp_in(SDL_Keycode k)
{
	switch (k) {
		case SDLK_W: joypad |= (1 << 4); break;
		case SDLK_Q: joypad |= (1 << 5); break;
		case SDLK_S: joypad |= (1 << 6); break;
		case SDLK_A: joypad |= (1 << 7); break;

		case SDLK_RIGHT: joypad |= (1 << 8); break;
		case SDLK_LEFT: joypad |= (1 << 9); break;
		case SDLK_DOWN: joypad |= (1 << 10); break;
		case SDLK_UP: joypad |= (1 << 11); break;

		case SDLK_RETURN: joypad |= (1 << 12); break;
		case SDLK_SPACE: joypad |= (1 << 13); break;
		case SDLK_X: joypad |= (1 << 14); break;
		case SDLK_Z: joypad |= (1 << 15); break;
	}
}

void Joypad::handle_joyp_out(SDL_Keycode k)
{
	switch (k) {
		case SDLK_W: joypad &= ~(1 << 4); break;
		case SDLK_Q: joypad &= ~(1 << 5); break;
		case SDLK_S: joypad &= ~(1 << 6); break;
		case SDLK_A: joypad &= ~(1 << 7); break;

		case SDLK_RIGHT: joypad &= ~(1 << 8); break;
		case SDLK_LEFT: joypad &= ~(1 << 9); break;
		case SDLK_DOWN: joypad &= ~(1 << 10); break;
		case SDLK_UP: joypad &= ~(1 << 11); break;

		case SDLK_RETURN: joypad &= ~(1 << 12); break;
		case SDLK_SPACE: joypad &= ~(1 << 13); break;
		case SDLK_X: joypad &= ~(1 << 14); break;
		case SDLK_Z: joypad &= ~(1 << 15); break;
	}
}

void Joypad::update_autoread()
{
	snes->bus.regs.joy1 = joypad;
}

void Joypad::update_shiftreg()
{
	shift_reg = joypad;
}

uint8_t Joypad::get_serinput()
{
	uint8_t tmp = shift_reg >> 15 & 1;
	shift_reg <<= 1;
	shift_reg |= 1;
	return tmp;
}
