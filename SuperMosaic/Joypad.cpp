#include "Joypad.h"
#include "SNES.h"

Controller::Controller(SNES* snes) : snes(snes)
{
}

SNESController::SNESController(SNES* snes) : Controller(snes)
{
	joypad = shift_reg = 0;
}


SNESMouse::SNESMouse(SNES* snes) : Controller(snes)
{
	mouse = 1 << 8;
	shift_reg = 0;
}

void SNESController::handle_ctrl_in(SDL_Event e)
{
	switch (e.key.key) {
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

void SNESController::handle_ctrl_out(SDL_Event e)
{
	switch (e.key.key) {
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

void SNESController::update_autoread()
{
	snes->bus.regs.joy1 = joypad;
}

void SNESController::update_shiftreg()
{
	shift_reg = joypad;
}

uint8_t SNESController::get_serinput()
{
	uint8_t tmp = shift_reg >> 15 & 1;
	shift_reg <<= 1;
	shift_reg |= 1;
	return tmp;
}

void SNESMouse::handle_ctrl_in(SDL_Event e)
{
	switch (e.type) {
		case SDL_EVENT_MOUSE_MOTION: {
			uint8_t x = static_cast<uint8_t>(e.motion.x);
			uint8_t y = static_cast<uint8_t>(e.motion.y);
			
			uint8_t cur_x = (mouse & 0x7F000000) >> 24;
			uint8_t cur_y = (mouse & 0x007F0000) >> 16;
			
			if (x <= cur_x)
				mouse |= (1 << 24);
			else
				mouse &= ~(1 << 16);

			if (y <= cur_y)
				mouse |= (1 << 16);
			else
				mouse &= ~(1 << 24);
			
			mouse = ~(mouse & 0x7F7F0000);

			mouse |= (x << 24);
			mouse |= (y << 16);
		}
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			mouse |= (1 << 14);
			break;
	}
}

void SNESMouse::handle_ctrl_out(SDL_Event e)
{
	switch (e.type) {
	case SDL_EVENT_MOUSE_BUTTON_UP:
		mouse &= ~(1 << 14);
		break;
	}
}

void SNESMouse::update_autoread()
{
	snes->bus.regs.joy1 = mouse & 0xFFFF;
}

void SNESMouse::update_shiftreg()
{
	shift_reg = mouse;
}

uint8_t SNESMouse::get_serinput()
{
	uint8_t tmp = shift_reg >> 31 & 1;
	shift_reg <<= 1;
	shift_reg |= 1;
	return tmp;
}