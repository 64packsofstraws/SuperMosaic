#pragma once
#include <cstdint>
#include <SDL3/SDL.h>

class SNES;

class Controller
{
protected:
	SNES* snes;

public:
	Controller(SNES* snes);

	virtual void handle_ctrl_in(SDL_Event e) = 0;
	virtual void handle_ctrl_out(SDL_Event e) = 0;

	virtual void update_autoread() = 0;
	virtual void update_shiftreg() = 0;

	virtual uint8_t get_serinput() = 0;
};

class SNESController : public Controller
{
public:
	uint16_t joypad;
	uint16_t shift_reg;

	SNESController(SNES* snes);

	void handle_ctrl_in(SDL_Event e);
	void handle_ctrl_out(SDL_Event e);

	void update_autoread();
	void update_shiftreg();

	uint8_t get_serinput();
};

class SNESMouse : public Controller
{
public:
	uint32_t mouse;
	uint32_t shift_reg;

	SNESMouse(SNES* snes);

	void handle_ctrl_in(SDL_Event e);
	void handle_ctrl_out(SDL_Event e);

	void update_autoread();
	void update_shiftreg();

	uint8_t get_serinput();
};
