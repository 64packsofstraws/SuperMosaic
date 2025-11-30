#include "PPU.h"
#include "SNES.h"

uint8_t PPU::read_reg(uint16_t addr)
{
	switch (addr) {
		case 0x2134:
			mdr = regs.mpyl;
			break;

		case 0x2135:
			mdr = regs.mpym;
			break;

		case 0x2136:
			mdr = regs.mpyh;
			break;

		case 0x2137:
			regs.ophct = dot;
			regs.opvct = scanline;
			counter_latch = true;
			break;

		case 0x2138: {
			uint8_t tmp = oam[internal_oamadd & 0x21F];
			internal_oamadd++;
			mdr = tmp;
		}
			break;

		case 0x2139:
			regs.vmdatalread = vram_latch & 0xFF;
			
			if (!(regs.vmain & 0x80)) {
				vram_latch = vram[vram_addr];
				vram_addr += vram_inc;
				vram_addr &= 0x7FFF;

				regs.vmaddh = (vram_addr >> 8) & 0xFF;
				regs.vmaddl = vram_addr & 0xFF;
			}
			
			mdr = regs.vmdatalread;
			break;

		case 0x213A:
			regs.vmdatahread = vram_latch >> 8;

			if (regs.vmain & 0x80) {
				vram_latch = vram[vram_addr];
				vram_addr += vram_inc;
				vram_addr &= 0x7FFF;

				regs.vmaddh = (vram_addr >> 8) & 0xFF;
				regs.vmaddl = vram_addr & 0xFF;
			}

			mdr = regs.vmdatahread;
			break;

		case 0x213B:
			regs.cgdataread = cgram[regs.cgadd];

			if (!cgreg_write) {
				cgreg_write = true;
				mdr = regs.cgdataread & 0xFF;
			}
			else {
				cgreg_write = false;
				regs.cgadd++;
				mdr = (regs.cgdataread >> 8) & 0x7F;
			}
			break;

		case 0x213C: {
			uint8_t tmp;
			if (!ophct_byte) tmp = regs.ophct & 0xFF;
			else tmp = (regs.ophct >> 8) & 0xFF;

			ophct_byte = !ophct_byte;
			mdr = tmp;
		}
			break;

		case 0x213D: {
			uint8_t tmp;
			if (!opvct_byte) tmp = regs.opvct & 0xFF;
			else tmp = (regs.opvct >> 8) & 0xFF;

			opvct_byte = !opvct_byte;
			mdr = tmp;
		}
			break;

		case 0x213E:
			mdr = regs.stat77;
			break;

		case 0x213F: {
			uint8_t tmp = regs.stat78 | (counter_latch << 6) | (interlace_frame << 7);
			counter_latch = false;
			opvct_byte = ophct_byte = false;
			mdr = tmp;
		}
	}

	return mdr;
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

			sprite_page0 = ((regs.objsel & 0x3) << 13) % 0x8000;
			sprite_page1 = (((((regs.objsel >> 3) & 0x3) + 1) << 12) + sprite_page0) % 0x8000;
			break;

		case 0x2102:
			regs.oamadd = (regs.oamadd & 0xFF00) | val;
			internal_oamadd = (regs.oamadd & 0x1FF) << 1;
			oam_reload = internal_oamadd;
			break;

		case 0x2103:
			regs.oamadd = (regs.oamadd & 0x00FF) | (val << 8);
			internal_oamadd = (regs.oamadd & 0x1FF) << 1;
			oam_reload = internal_oamadd;
			break;

		case 0x2104:
			if (internal_oamadd < 544) {
				regs.oamdata = val;

				if (!(internal_oamadd & 1)) oam_latch = val;

				if (internal_oamadd < 0x200 && (internal_oamadd & 1)) {
					oam[internal_oamadd - 1] = oam_latch;
					oam[internal_oamadd] = val;
				}

				if (internal_oamadd >= 0x200) oam[internal_oamadd] = val;

				internal_oamadd++;

				if (internal_oamadd == 544) regs.stat77 |= 0x40;
				else regs.stat77 &= ~0x40;
			}
			break;

		case 0x2105:
			regs.bgmode = val;

			switch (regs.bgmode & 0x7) {
				case 0:
					bg[0].bpp = 2;
					bg[1].bpp = 2;
					bg[2].bpp = 2;
					bg[3].bpp = 2;
					num_of_bgs = 4;
					break;

				case 1:
					bg[0].bpp = 4;
					bg[1].bpp = 4;
					bg[2].bpp = 2;
					num_of_bgs = 3;
					break;

				case 2:
					bg[0].bpp = 4;
					bg[1].bpp = 4;
					num_of_bgs = 2;
					break;

				case 3:
					bg[0].bpp = 8;
					bg[1].bpp = 4;
					num_of_bgs = 2;
					break;

				case 4:
					bg[0].bpp = 8;
					bg[1].bpp = 2;
					num_of_bgs = 2;
					break;

				case 5:
					bg[0].bpp = 4;
					bg[1].bpp = 2;
					num_of_bgs = 2;
					break;

				case 6:
					bg[0].bpp = 4;
					num_of_bgs = 1;
					break;

				case 7:
					bg[0].bpp = 8;
					num_of_bgs = 1;
					break;
			}
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
			vram_latch = vram[vram_addr];
			break;

		case 0x2117:
			regs.vmaddh = val & 0x7F;
			vram_addr = (vram_addr & 0x00FF) | (regs.vmaddh << 8);
			vram_latch = vram[vram_addr];
			break;

		case 0x2118:
			if (regs.inidisp & 0x80 || stage == VBLANK) {
				regs.vmdatal = val;
				vram[vram_addr] = (vram[vram_addr] & 0xFF00) | regs.vmdatal;

				if (!(regs.vmain & 0x80)) {
					vram_addr += vram_inc;
					vram_addr &= 0x7FFF;

					regs.vmaddh = (vram_addr >> 8) & 0xFF;
					regs.vmaddl = vram_addr & 0xFF;
				}
			}
			break;

		case 0x2119:
			if (regs.inidisp & 0x80 || stage == VBLANK) {
				regs.vmdatah = val;
				vram[vram_addr] = (vram[vram_addr] & 0x00FF) | (regs.vmdatah << 8);

				if (regs.vmain & 0x80) {
					vram_addr += vram_inc;
					vram_addr &= 0x7FFF;

					regs.vmaddh = (vram_addr >> 8) & 0xFF;
					regs.vmaddl = vram_addr & 0xFF;
				}
			}
			break;

		case 0x211A:
			regs.m7sel = val;
			break;

		case 0x211B: {
			static int write_twice = 0;

			regs.m7a = (val << 8) | m7_latch;
			m7_latch = val;
			write_twice++;

			if (write_twice == 2) {
				int16_t a = regs.m7a;
				int8_t b = regs.m7b & 0xFF;

				int result = (a * b) & 0xFFFFFF;

				regs.mpyh = (result >> 16) & 0xFF;
				regs.mpym = (result >> 8) & 0xFF;
				regs.mpyl = result & 0xFF;
				write_twice = 0;
			}
		}
			break;

		case 0x211C: {
			regs.m7b = (val << 8) | m7_latch;
			m7_latch = val;

			int16_t a = regs.m7a;
			int8_t b = regs.m7b & 0xFF;

			int result = (a * b) & 0xFFFFFF;

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
			for (int i = 0; i < 4; i++)
				bg[i].in_main = (regs.tm & (1 << i)) != 0;
			break;

		case 0x212D:
			regs.ts = val;
			for (int i = 0; i < 4; i++)
				bg[i].in_sub = (regs.ts & (1 << i)) != 0;
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

		case 0x2132: {
			regs.coldata = val;
			uint8_t intensity = val & 0x1F;

			if ((regs.coldata >> 5) & 1) fc.r = intensity;
			if ((regs.coldata >> 6) & 1) fc.g = intensity;
			if ((regs.coldata >> 7) & 1) fc.b = intensity;
		}
			break;

		case 0x2133:
			regs.setini = val;

			vblank_scanline = (regs.setini & 0x4) ? 240 : 225;
			break;
	}
}