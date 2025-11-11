#include "Bus.h"
#include "SNES.h"

uint8_t Bus::read_regs(uint16_t addr)
{
	if (addr >= 0x2140 && addr <= 0x217F) {
		uint8_t idx = (addr - 0x2140) % 4;
		uint8_t tmp = regs.apuio[idx];

		switch (idx) {
			case 0: regs.apuio[idx] = 0xAA; break;
			case 1: regs.apuio[idx] = 0xBB;	break;
			default: regs.apuio[idx] = 0x0;
		}

		return tmp;
	}

	switch (addr) {
		case 0x2180:
			return wram[wram_addr++];

		case 0x4016:
			return snes->ctrlr->get_serinput();

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

		case 0x4210: {
			uint8_t tmp = regs.rdnmi ^ (snes->ppu.get_vblank_flag() << 7);
			snes->ppu.set_vblank_flag(false);
			regs.rdnmi &= 0x7F;
			return tmp;
		}

		case 0x4211: {
			uint8_t tmp = regs.timeup;
			regs.timeup &= 0x7F;
			return tmp;
		}
		
		case 0x4212:
			return regs.hvbjoy;

	}

	return mdr;
}

void Bus::write_regs(uint16_t addr, uint8_t val)
{
	if (addr >= 0x2140 && addr <= 0x217F) {
		regs.apuio[(addr - 0x2140) % 4] = val;
		return;
	}

	switch (addr) {
		case 0x2180:
			wram[wram_addr++] = val;
			break;

		case 0x2181:
			regs.wmaddl = val;
			wram_addr = (wram_addr & 0xFFFF00) | regs.wmaddl;
			break;

		case 0x2182:
			regs.wmaddm = val;
			wram_addr = (wram_addr & 0xFF00FF) | (regs.wmaddm << 8);
			break;

		case 0x2183:
			regs.wmaddh = val & 0x1;
			wram_addr = (wram_addr & 0x00FFFF) | (regs.wmaddh << 16);
			break;

		case 0x4016: {
			static bool prev_strobe = 1;

			if (val & 1 && prev_strobe == 0) {
				snes->ctrlr->update_shiftreg();
			}

			prev_strobe = val & 1;
		}
		   break;

		case 0x4200:
			regs.nmitimen = val;
			snes->ppu.set_nmi_enable(regs.nmitimen & 0x80);
			break;

		case 0x4201: {
			static bool prev_latch = 0;

			if (val & 1 && prev_latch == 0) {
				snes->ppu.latch_hv();
			}

			prev_latch = val & 1;
		}
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

		case 0x4207:
			regs.htime = (regs.vtime & 0xFF00) | val;
			break;

		case 0x4208:
			regs.htime = (regs.vtime & 0x00FF) | ((val & 1) << 8);
			break;

		case 0x4209:
			regs.vtime = (regs.vtime & 0xFF00) | val;
			break;

		case 0x420A:
			regs.vtime = (regs.vtime & 0x00FF) | ((val & 1) << 8);
			break;

		case 0x420B:
			regs.mdmaen = val;

			for (int i = 0; i < 8; i++) {
				if (regs.mdmaen & (1 << i)) snes->dma.dma_transfer(i);
			}
			break;

		case 0x420C:
			snes->dma.hdmaen = val;
			snes->dma.hdma_init();
			break;

		case 0x420D:
			regs.memsel = val & 1;
			break;
	}
}

Bus::Bus(SNES* snes) : snes(snes), wram(0x20000, 0)
{
	memset(&regs, 0, sizeof(regs));
	regs.apuio[0] = 0xAA;
	regs.apuio[1] = 0;
	regs.apuio[2] = 0;
	regs.apuio[3] = 0;

	shift_reg = 0;
	wram_addr = 0;
	mdr = 0;
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
	if (bank >= 0x70 && bank <= 0x7D && lower_word < 0x8000) {
		snes->cpu.tick_components(8);
		mdr = snes->cart->read_ram(addr);
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
	if (bank >= 0x70 && bank <= 0x7D && lower_word < 0x8000) {
		snes->cpu.tick_components(8);
		snes->cart->write_ram(addr, val);
	}
	if (bank >= 0x7E && bank <= 0x7F) {
		snes->cpu.tick_components(8);
		wram[addr - 0x7E0000] = val;
	}

	mdr = val;
}

uint8_t Bus::read_noticks(uint32_t addr)
{
	uint8_t bank = (addr >> 16) & 0xFF;
	uint16_t lower_word = addr & 0xFFFF;

	if (bank >= 0x0 && bank <= 0x3F || bank >= 0x80 && bank <= 0xBF) {
		if (lower_word < 0x2000) {
			mdr = wram[lower_word];
		}
		else if (lower_word >= 0x2100 && lower_word <= 0x213F) {
			mdr = snes->ppu.read_reg(lower_word);
		}
		else if (lower_word >= 0x2140 && lower_word <= 0x421F) {
			mdr = read_regs(lower_word);
		}
		else if (lower_word >= 0x4300 && lower_word <= 0x437F) {
			mdr = snes->dma.read_reg(lower_word);
		}
	}
	if (bank >= 0x70 && bank <= 0x7D && lower_word < 0x8000) {
		mdr = snes->cart->read_ram(addr);
	}
	if (bank >= 0x7E && bank <= 0x7F) {
		mdr = wram[addr - 0x7E0000];
	}
	if (snes->cart->in_range(addr)) {
		mdr = snes->cart->read(addr);
	}

	return mdr;
}

void Bus::write_noticks(uint32_t addr, uint8_t val)
{
	uint8_t bank = (addr >> 16) & 0xFF;
	uint16_t lower_word = addr & 0xFFFF;

	if (bank >= 0x0 && bank <= 0x3F || bank >= 0x80 && bank <= 0xBF) {
		if (lower_word < 0x2000) {
			wram[lower_word] = val;
		}
		else if (lower_word >= 0x2100 && lower_word <= 0x213F) {
			snes->ppu.write_reg(lower_word, val);
		}
		else if (lower_word >= 0x2140 && lower_word <= 0x421F) {
			write_regs(lower_word, val);
		}
		else if (lower_word >= 0x4300 && lower_word <= 0x437F) {
			snes->dma.write_reg(lower_word, val);
		}
	}
	if (bank >= 0x70 && bank <= 0x7D && lower_word < 0x8000) {
		snes->cpu.tick_components(8);
		snes->cart->write_ram(addr, val);
	}
	if (bank >= 0x7E && bank <= 0x7F) {
		wram[addr - 0x7E0000] = val;
	}

	mdr = val;
}
