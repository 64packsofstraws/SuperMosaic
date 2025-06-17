#include "PPU.h"
#include "SNES.h"

uint8_t PPU::read_reg(uint16_t addr)
{
	switch (addr) {
		case 0x2134:
			return regs.mpyl;

		case 0x2135:
			return regs.mpym;

		case 0x2136:
			return regs.mpyh;

		case 0x2137:
			return regs.slhv;

		case 0x2138:
			return regs.oamdataread;

		case 0x2139:
			regs.vmdatalread = vram[vram_addr] & 0xFF;
			
			if (!(regs.vmain & 0x80)) {
				vram_addr += vram_inc;
				vram_addr &= 0x7FFF;
			}
			
			return regs.vmdatalread;

		case 0x213A:
			regs.vmdatahread = vram[vram_addr] >> 8;

			if (regs.vmain & 0x80) {
				vram_addr += vram_inc;
				vram_addr &= 0x7FFF;
			}

			return regs.vmdatahread;

		case 0x213B:
			regs.cgdataread = cgram[regs.cgadd];

			if (!cgreg_write) {
				cgreg_write = true;
				return regs.cgdataread & 0xFF;
			}
			else {
				cgreg_write = false;
				regs.cgadd++;
				return (regs.cgdataread >> 8) & 0x7F;
			}
	}
	return 0;
}

void PPU::write_reg(uint16_t addr, uint8_t val)
{
	static uint8_t bgofs_latch = 0;
	static uint8_t bghofs_latch = 0;

	switch (addr) {
		case 0x2100:
			regs.inidisp = val;
			break;

		case 0x2101:
			regs.objsel = val;
			break;

		case 0x2102:
			regs.oamaddl = val;
			break;

		case 0x2103:
			regs.oamaddh = val;
			break;

		case 0x2104:
			regs.oamdata = val;
			break;

		case 0x2105:
			regs.bgmode = val;
			break;

		case 0x2106:
			regs.mosaic = val;
			break;

		case 0x2107:
			regs.bg1sc = val;

			bg[0].tilemap_base = (regs.bg1sc & 0xFC) << 8;

			switch (regs.bg1sc & 3) {
			case 0:
				bg[0].tilemap_sizex = 32;
				bg[0].tilemap_sizey = 32;
				break;

			case 1:
				bg[0].tilemap_sizex = 64;
				bg[0].tilemap_sizey = 32;
				break;

			case 2:
				bg[0].tilemap_sizex = 32;
				bg[0].tilemap_sizey = 64;
				break;

			case 3:
				bg[0].tilemap_sizex = 64;
				bg[0].tilemap_sizey = 64;
			}
			break;

		case 0x2108:
			regs.bg2sc = val;

			bg[1].tilemap_base = (regs.bg2sc & 0xFC) << 8;

			switch (regs.bg1sc & 3) {
			case 0:
				bg[1].tilemap_sizex = 32;
				bg[1].tilemap_sizey = 32;
				break;

			case 1:
				bg[1].tilemap_sizex = 64;
				bg[1].tilemap_sizey = 32;
				break;

			case 2:
				bg[1].tilemap_sizex = 32;
				bg[1].tilemap_sizey = 64;
				break;

			case 3:
				bg[1].tilemap_sizex = 64;
				bg[1].tilemap_sizey = 64;
			}
			break;

		case 0x2109:
			regs.bg3sc = val;

			bg[2].tilemap_base = (regs.bg3sc & 0xFC) << 8;

			switch (regs.bg1sc & 3) {
			case 0:
				bg[2].tilemap_sizex = 32;
				bg[2].tilemap_sizey = 32;
				break;

			case 1:
				bg[2].tilemap_sizex = 64;
				bg[2].tilemap_sizey = 32;
				break;

			case 2:
				bg[2].tilemap_sizex = 32;
				bg[2].tilemap_sizey = 64;
				break;

			case 3:
				bg[2].tilemap_sizex = 64;
				bg[2].tilemap_sizey = 64;
			}
			break;

		case 0x210A:
			regs.bg4sc = val;

			bg[3].tilemap_base = (regs.bg4sc & 0xFC) << 8;

			switch (regs.bg1sc & 3) {
			case 0:
				bg[3].tilemap_sizex = 32;
				bg[3].tilemap_sizey = 32;
				break;

			case 1:
				bg[3].tilemap_sizex = 64;
				bg[3].tilemap_sizey = 32;
				break;

			case 2:
				bg[3].tilemap_sizex = 32;
				bg[3].tilemap_sizey = 64;
				break;

			case 3:
				bg[3].tilemap_sizex = 64;
				bg[3].tilemap_sizey = 64;
			}
			break;

		case 0x210B:
			regs.bg12nba = val;

			bg[0].tileset_base = (regs.bg12nba & 0xF) << 12;
			bg[1].tileset_base = (regs.bg12nba & 0xF0) << 8;
			break;

		case 0x210C:
			regs.bg34nba = val;

			bg[2].tileset_base = (regs.bg34nba & 0xF) << 12;
			bg[3].tileset_base = (regs.bg34nba & 0xF0) << 8;
			break;

		case 0x210D:
			bg[0].scrollx = (val << 8) | (bgofs_latch & ~7) | (bghofs_latch & 7);
			bgofs_latch = val;
			bghofs_latch = val;
			break;

		case 0x210E:
			bg[0].scrolly = (val << 8) | bgofs_latch;
			bgofs_latch = val;
			break;

		case 0x210F:
			bg[1].scrollx = (val << 8) | (bgofs_latch & ~7) | (bghofs_latch & 7);
			bgofs_latch = val;
			bghofs_latch = val;
			break;

		case 0x2110:
			bg[1].scrolly = (val << 8) | bgofs_latch;
			bgofs_latch = val;
			break;

		case 0x2111:
			bg[2].scrollx = (val << 8) | (bgofs_latch & ~7) | (bghofs_latch & 7);
			bgofs_latch = val;
			bghofs_latch = val;
			break;

		case 0x2112:
			bg[2].scrolly = (val << 8) | bgofs_latch;
			bgofs_latch = val;
			break;

		case 0x2113:
			bg[3].scrollx = (val << 8) | (bgofs_latch & ~7) | (bghofs_latch & 7);
			bgofs_latch = val;
			bghofs_latch = val;
			break;

		case 0x2114:
			bg[3].scrolly = (val << 8) | bgofs_latch;
			bgofs_latch = val;
			break;

		case 0x2115:
			regs.vmain = val;

			switch (regs.vmain & 0x3) {
			case 0: vram_inc = 1; break;
			case 1: vram_inc = 32; break;
			case 2: case 3: vram_inc = 128; break;
			}
			break;

		case 0x2116:
			regs.vmaddl = val;
			vram_addr = (vram_addr & 0x7F00) | regs.vmaddl;
			break;

		case 0x2117:
			regs.vmaddh = val & 0x7F;
			vram_addr = (vram_addr & 0x00FF) | (regs.vmaddh << 8);
			break;

		case 0x2118:
			regs.vmdatal = val;
			vram[vram_addr] = (vram[vram_addr] & 0xFF00) | regs.vmdatal;

			if (!(regs.vmain & 0x80)) {
				vram_addr += vram_inc;
				vram_addr &= 0x7FFF;
			}
			break;

		case 0x2119:
			regs.vmdatah = val;
			vram[vram_addr] = (vram[vram_addr] & 0x00FF) | (regs.vmdatah << 8);

			if (regs.vmain & 0x80) {
				vram_addr += vram_inc;
				vram_addr &= 0x7FFF;
			}
			break;

		case 0x211A:
			regs.m7sel = val;
			break;

		case 0x211B: {
				static bool reg_write = false;

				if (!reg_write) {
					regs.m7a = (regs.m7a & 0xFF00) | val;
					reg_write = true;
				}
				else {
					regs.m7a = (regs.m7a & 0x00FF) | (val << 8);
					reg_write = false;
				}

				int result = regs.m7a * regs.m7b;

				regs.mpyh = (result >> 16) & 0xFF;
				regs.mpym = (result >> 8) & 0xFF;
				regs.mpyl = result & 0xFF;
		}
				   break;

		case 0x211C: {
					   regs.m7b = val;
					   int result = regs.m7a * regs.m7b;

					   regs.mpyh = (result >> 16) & 0xFF;
					   regs.mpym = (result >> 8) & 0xFF;
					   regs.mpyl = result & 0xFF;
		}
				   break;

		case 0x211D:
			regs.m7c = val;
			break;

		case 0x211E:
			regs.m7d = val;
			break;

		case 0x211F:
			regs.m7x = val;
			break;

		case 0x2120:
			regs.m7y = val;
			break;

		case 0x2121:
			regs.cgadd = val;
			cgreg_write = false;
			break;

		case 0x2122: {
				if (!cgreg_write) {
					cgram[regs.cgadd] = (cgram[regs.cgadd] & 0xFF00) | val;
					cgreg_write = true;
				}
				else {
					cgram[regs.cgadd] = (cgram[regs.cgadd] & 0x00FF) | (val << 8);
					cgreg_write = false;
					regs.cgadd++;
				}
		}
				   break;

		case 0x2123:
			regs.w12sel = val;
			break;

		case 0x2124:
			regs.w34sel = val;
			break;

		case 0x2125:
			regs.wobjsel = val;
			break;

		case 0x2126:
			regs.wh0 = val;
			break;

		case 0x2127:
			regs.wh1 = val;
			break;

		case 0x2128:
			regs.wh2 = val;
			break;

		case 0x2129:
			regs.wh3 = val;
			break;

		case 0x212A:
			regs.wbglog = val;
			break;

		case 0x212B:
			regs.wobjlog = val;
			break;

		case 0x212C:
			regs.tm = val;
			break;

		case 0x212D:
			regs.ts = val;
			break;

		case 0x212E:
			regs.tmw = val;
			break;

		case 0x212F:
			regs.tsw = val;
			break;

		case 0x2130:
			regs.cgwsel = val;
			break;

		case 0x2131:
			regs.cgadsub = val;
			break;

		case 0x2132:
			regs.coldata = val;
			break;

		case 0x2133:
			regs.setini = val;

			vblank_scanline = (regs.setini & 0x4) ? 240 : 225;
			break;
	}
}