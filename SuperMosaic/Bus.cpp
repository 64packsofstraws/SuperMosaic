#include "Bus.h"
#include "SNES.h"

uint8_t Bus::read_regs(uint16_t addr)
{
	switch (addr) {
		case 0x2140:
		case 0x2141:
		case 0x2142:
		case 0x2143:
			return regs.apuio[addr & 0x3];

		case 0x2180:
			return wram[wram_addr++];

		case 0x4016:
			return regs.joyser0;

		case 0x4214:
			return regs.rddivl;

		case 0x4215:
			return regs.rddivh;

		case 0x4216:
			return regs.rdmpyl;

		case 0x4217:
			return regs.rdmpyh;

		case 0x4218:
			return regs.joy1 & 0xFF;

		case 0x4219:
			return (regs.joy1 >> 8) & 0xFF;

		case 0x421A:
		case 0x421B:
		case 0x421C:
		case 0x421D:
		case 0x421E:
		case 0x421F:
			return 0;

		case 0x4210:
			uint8_t tmp = regs.rdnmi ^ (snes->ppu.get_vblank_flag() << 7);
			snes->ppu.set_vblank_flag(false);
			regs.rdnmi &= 0x7F;
			return tmp;
	}

	return mdr;
}

void Bus::write_regs(uint16_t addr, uint8_t val)
{
	switch (addr) {
		case 0x2140:
		case 0x2141:
		case 0x2142:
		case 0x2143:
			regs.apuio[addr & 0x3] = val;
			break;

		case 0x2180:
			wram[wram_addr++] = val;
			break;

		case 0x2181:
			regs.wmaddh = val & 0x1;
			wram_addr = (wram_addr & 0x00FFFF) | (regs.wmaddh << 16);
			break;

		case 0x2182:
			regs.wmaddm = val;
			wram_addr = (wram_addr & 0xFF00FF) | (regs.wmaddm << 4);
			break;

		case 0x2183:
			regs.wmaddl = val;
			wram_addr = (wram_addr & 0xFFFF00) | regs.wmaddl;
			break;

		case 0x4016:
			regs.joyout = val;
			break;

		case 0x4200:
			regs.nmitimen = val;
			snes->ppu.set_nmi_enable(regs.nmitimen & 0x80);
			break;

		case 0x4201:
			regs.wrio = val;
			break;

		case 0x4202:
			regs.wrmpya = val;
			break;

		case 0x4203: {
			regs.wrmpyb = val;
		
			uint16_t result = regs.wrmpya * regs.wrmpyb;

			regs.rdmpyl = result & 0xFF;
			regs.rdmpyh = result >> 8;
		}
			break;

		case 0x4204:
			regs.wrdivl = val;
			break;

		case 0x4205:
			regs.wrdivh = val;
			break;

		case 0x4206: {
			regs.wrdivb = val;

			uint16_t dividend = (regs.wrdivh << 8) | regs.wrdivl;
			uint8_t divisor = regs.wrdivb;

			uint16_t quotient;
			uint16_t remainder;

			if (divisor > 0) {
				quotient = dividend / divisor;
				remainder = dividend % divisor;
			}
			else {
				quotient = 0xFFFF;
				remainder = dividend;
			}

			regs.rddivh = quotient >> 8;
			regs.rddivl = quotient & 0xFF;

			regs.rdmpyh = remainder >> 8;
			regs.rdmpyl = remainder & 0xFF;
		}
			break;

		case 0x420B:
			regs.mdmaen = val;

			for (int i = 0; i < 8; i++) {
				if (regs.mdmaen & (1 << i)) snes->dma.dma_transfer(i);
			}
			break;

		case 0x420C:
			regs.hdmaen = val;
			break;

		case 0x420D:
			regs.memsel = val & 1;
			break;
	}
}

Bus::Bus(SNES* snes) : snes(snes), wram(0x20000, 0)
{
	memset(&regs, 0, sizeof(regs));
	wram_addr = 0;
	mdr = 0;
}

void Bus::handle_joyp_in(SDL_Keycode k)
{
	switch (k) {
		case SDLK_W: regs.joy1 |= (1 << 4); break;
		case SDLK_Q: regs.joy1 |= (1 << 5); break;
		case SDLK_S: regs.joy1 |= (1 << 6); break;
		case SDLK_A: regs.joy1 |= (1 << 7); break;

		case SDLK_RIGHT: regs.joy1 |= (1 << 8); break;
		case SDLK_LEFT: regs.joy1 |= (1 << 9); break;
		case SDLK_DOWN: regs.joy1 |= (1 << 10); break;
		case SDLK_UP: regs.joy1 |= (1 << 11); break;

		case SDLK_RETURN: regs.joy1 |= (1 << 12); break;
		case SDLK_SPACE: regs.joy1 |= (1 << 13); break;
		case SDLK_X: regs.joy1 |= (1 << 14); break;
		case SDLK_Z: regs.joy1 |= (1 << 15); break;
	}
}

void Bus::handle_joyp_out(SDL_Keycode k)
{
	switch (k) {
		case SDLK_W: regs.joy1 &= ~(1 << 4); break;
		case SDLK_Q: regs.joy1 &= ~(1 << 5); break;
		case SDLK_S: regs.joy1 &= ~(1 << 6); break;
		case SDLK_A: regs.joy1 &= ~(1 << 7); break;

		case SDLK_RIGHT: regs.joy1 &= ~(1 << 8); break;
		case SDLK_LEFT: regs.joy1 &= ~(1 << 9); break;
		case SDLK_DOWN: regs.joy1 &= ~(1 << 10); break;
		case SDLK_UP: regs.joy1 &= ~(1 << 11); break;

		case SDLK_RETURN: regs.joy1 &= ~(1 << 12); break;
		case SDLK_SPACE: regs.joy1 &= ~(1 << 13); break;
		case SDLK_X: regs.joy1 &= ~(1 << 14); break;
		case SDLK_Z: regs.joy1 &= ~(1 << 15); break;
	}
}

uint8_t Bus::read(uint32_t addr)
{
	uint8_t bank = (addr >> 16) & 0xFF;
	uint16_t lower_word = addr & 0xFFFF;

	if (bank >= 0x0 && bank <= 0x3F || bank >= 0x80 && bank <= 0xBF) {
		if (lower_word < 0x2000) {
			snes->cpu.tick_components(8);
			mdr = wram[lower_word];
		}
		else if (lower_word >= 0x2100 && lower_word <= 0x213F) {
			snes->cpu.tick_components(6);
			mdr = snes->ppu.read_reg(lower_word);
		}
		else if (lower_word >= 0x2140 && lower_word <= 0x421F) {
			snes->cpu.tick_components(lower_word == 0x4016 || lower_word == 0x4017 ? 12 : 6);
			mdr = read_regs(lower_word);
		}
		else if (lower_word >= 0x4300 && lower_word <= 0x437F) {
			snes->cpu.tick_components(6);
			mdr = snes->dma.read_reg(lower_word);
		}
	}
	if (bank >= 0x7E && bank <= 0x7F) {		
		snes->cpu.tick_components(8);
		mdr = wram[addr - 0x7E0000];
	}
	if (snes->cart->in_range(addr)) {
		snes->cpu.tick_components(snes->cart->get_rom_speed() && regs.memsel && bank >= 0x80 ? 6 : 8);
		mdr = snes->cart->read(addr);
	}

	return mdr;
}

void Bus::write(uint32_t addr, uint8_t val)
{
	uint8_t bank = (addr >> 16) & 0xFF;
	uint16_t lower_word = addr & 0xFFFF;

	if (bank >= 0x0 && bank <= 0x3F || bank >= 0x80 && bank <= 0xBF) {
		if (lower_word < 0x2000) {
			snes->cpu.tick_components(8);
			wram[lower_word] = val;
		}
		else if (lower_word >= 0x2100 && lower_word <= 0x213F) {
			snes->cpu.tick_components(6);
			snes->ppu.write_reg(lower_word, val);
		}
		else if (lower_word >= 0x2140 && lower_word <= 0x421F) {
			snes->cpu.tick_components(lower_word == 0x4016 || lower_word == 0x4017 ? 12 : 6);
			write_regs(lower_word, val);
		}
		else if (lower_word >= 0x4300 && lower_word <= 0x437F) {
			snes->cpu.tick_components(6);
			snes->dma.write_reg(lower_word, val);
		}
	}
	if (bank >= 0x7E && bank <= 0x7F) {
		snes->cpu.tick_components(8);
		wram[addr - 0x7E0000] = val;
	}

	mdr = val;
}