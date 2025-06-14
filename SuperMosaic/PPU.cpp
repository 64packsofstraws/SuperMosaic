#include "PPU.h"
#include "SNES.h"

void PPU::get_priority_m0()
{
	for (int i = 0; i < 4; i++) {
		if (regs.tm & (1 << i))
			priority_vec.push_back(bg[i]);
	}
	if (priority_vec.size() < 2) return;

	for (int i = 0; i <= priority_vec.size() / 2; i += 2) {
		uint16_t tmap_base = priority_vec[i].tilemap_base & 0x7FFF;
		uint16_t tmap_base_next = priority_vec[i + 1].tilemap_base & 0x7FFF;

		uint16_t tmap_idx = tmap_base + ((y / 8) * priority_vec[i].tilemap_sizex + (x / 8));
		uint16_t tmap_idx_next = tmap_base_next + ((y / 8) * priority_vec[i + 1].tilemap_sizex + (x / 8));

		bool priority = (vram[tmap_idx] >> 13) & 1;
		bool priority_next = (vram[tmap_idx_next] >> 13) & 1;

		if (priority < priority_next)
			std::swap(priority_vec[i], priority_vec[i + 1]);
	}
}

void PPU::render_bgpixel_m0()
{
	for (auto i = priority_vec.begin(); i != priority_vec.end(); i++) {
		uint16_t tmap_base = i->tilemap_base & 0x7FFF;
		uint16_t tset_base = i->tileset_base & 0x7FFF;

		uint16_t tmap_idx = tmap_base + ((y / 8) * i->tilemap_sizex + (x / 8));

		uint16_t tset_idx = tset_base + (vram[tmap_idx] & 0x3FF);

		uint16_t plane_idx = tset_idx * 8 + (y % 8);
		uint8_t p0 = vram[plane_idx] & 0xFF;
		uint8_t p1 = (vram[plane_idx] >> 8) & 0xFF;

		bool b0 = ( p0 & ( 0x80 >> (x % 8) )) != 0;
		bool b1 = ( p1 & ( 0x80 >> (x % 8) )) != 0;

		uint8_t pal_idx = (b1 << 1) | b0;

		if (!pal_idx && i != priority_vec.end() - 1) continue;

		uint8_t tmap_pal = (vram[tmap_idx] >> 10) & 0x7;
		uint16_t entry = (!pal_idx) ? cgram[0] : cgram[32 * i->num + 4 * tmap_pal + pal_idx];

		framebuf[idx(x, y)] = to_rgb888(entry);
		x++;
		break;
	}

	priority_vec.clear();
}

void PPU::get_priority_m1()
{
	if (regs.tm & 0x1) priority_vec.push_back(bg[0]);
	if (regs.tm & 0x2) priority_vec.push_back(bg[1]);

	if (priority_vec.size() == 2) {
		uint16_t tmap_base = priority_vec[0].tilemap_base & 0x7FFF;
		uint16_t tmap_base_next = priority_vec[1].tilemap_base & 0x7FFF;

		uint16_t tmap_idx = tmap_base + ((y / 8) * priority_vec[0].tilemap_sizex + (x / 8));
		uint16_t tmap_idx_next = tmap_base_next + ((y / 8) * priority_vec[1].tilemap_sizex + (x / 8));

		bool priority = (vram[tmap_idx] >> 13) & 1;
		bool priority_next = (vram[tmap_idx_next] >> 13) & 1;

		if (priority < priority_next)
			std::swap(priority_vec[0], priority_vec[1]);
	}

	if (regs.tm & 0x3) {
		if (regs.bgmode & 0x8) priority_vec.insert(priority_vec.begin(), bg[2]);
		else priority_vec.push_back(bg[2]);
	}
}

void PPU::render_bgpixel_m1()
{
	for (auto i = priority_vec.begin(); i != priority_vec.end(); i++) {
		uint16_t tmap_base = i->tilemap_base & 0x7FFF;
		uint16_t tset_base = i->tileset_base & 0x7FFF;

		uint16_t tmap_idx = tmap_base + ((y / 8) * i->tilemap_sizex + (x / 8));

		uint16_t tset_idx = tset_base + (vram[tmap_idx] & 0x3FF);

		uint16_t plane_idx = tset_idx * 16 + (y % 8);
		uint8_t p0 = vram[plane_idx] & 0xFF;
		uint8_t p1 = (vram[plane_idx] >> 8) & 0xFF;
		uint8_t p2 = vram[plane_idx + 8] & 0xFF;
		uint8_t p3 = (vram[plane_idx + 8] >> 8) & 0xFF;

		bool b0 = ( p0 & ( 0x80 >> (x % 8) )) != 0;
		bool b1 = ( p1 & ( 0x80 >> (x % 8) )) != 0;
		bool b2 = ( p2 & ( 0x80 >> (x % 8) )) != 0;
		bool b3 = ( p3 & ( 0x80 >> (x % 8) )) != 0;

		uint8_t pal_idx = (b1 << 1) | b0;
		pal_idx |= (b3 << 3) | (b2 << 2);

		if (!pal_idx && i != priority_vec.end() - 1) continue;

		uint8_t tmap_pal = (vram[tmap_idx] >> 10) & 0x7;
		uint16_t entry = (!pal_idx) ? cgram[0] : cgram[16 * tmap_pal + pal_idx];

		framebuf[idx(x, y)] = to_rgb888(entry);
		x++;
		break;
	}

	priority_vec.clear();
}

SDL_Color PPU::to_rgb888(uint16_t rgb)
{
	uint8_t r = rgb & 0x3F;
	uint8_t g = (rgb >> 5) & 0x3F;
	uint8_t b = (rgb >> 10) & 0x3F;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	return { r, g, b, 255 };
}

PPU::PPU(SNES* snes) : snes(snes), vram(0x8000, 0), cgram(256, 0), framebuf(256 * 224, {0, 0, 0})
{
	memset(&regs, 0, sizeof(regs));
	memset(&bg, 0, sizeof(bg));

	bg[0].num = 0;
	bg[1].num = 1;
	bg[2].num = 2;
	bg[3].num = 3;

	mr_table[0].get_priority = &PPU::get_priority_m0;
	mr_table[0].render_bgp = &PPU::render_bgpixel_m0;
	mr_table[1].get_priority = &PPU::get_priority_m1;
	mr_table[1].render_bgp = &PPU::render_bgpixel_m1;

	mdr = 0;
	vram_addr = vram_inc = 0;
	cgreg_write = false;

	vblank_scanline = 224;
	vblank_flag = nmi_enable = false;
	dot = scanline = mclock_dot = 0;

	x = y = 0;
#ifndef DEBUG
	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer("SuperMosaic", 256 * SCALE, 224 * SCALE, 0, &win, &ren);
	SDL_SetRenderVSync(ren, 1);

	tex = SDL_CreateTexture(
		ren,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		256 * SCALE,
		224 * SCALE
	);

	elapsed_tick = SDL_GetTicks();
#endif
}

PPU::~PPU()
{
#ifndef DEBUG
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
#endif
}

bool PPU::get_vblank_flag() const
{
	return vblank_flag;
}

void PPU::set_vblank_flag(bool val)
{
	vblank_flag = val;
}

void PPU::set_nmi_enable(bool val)
{
	nmi_enable = val;
}

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

		case 0x210D: {
			static bool reg_write = false;
			regs.bg1hofs = val;

			if (!reg_write) {
				bg[0].scrollx = (bg[0].scrollx & 0xFF00) | regs.bg1hofs;
				reg_write = true;
			}
			else {
				bg[0].scrollx = (bg[0].scrollx & 0x00FF) | (regs.bg1hofs << 8);
				reg_write = false;
			}
		}
			break;

		case 0x210E: {
				static bool reg_write = false;
				regs.bg1vofs = val;

				if (!reg_write) {
					bg[0].scrolly = (bg[0].scrolly & 0xFF00) | regs.bg1vofs;
					reg_write = true;
				}
				else {
					bg[0].scrolly = (bg[0].scrolly & 0x00FF) | (regs.bg1vofs << 8);
					reg_write = false;
				}
		}
			break;

		case 0x210F: {
			static bool reg_write = false;
			regs.bg2hofs = val;

			if (!reg_write) {
				bg[1].scrollx = (bg[1].scrollx & 0xFF00) | regs.bg2hofs;
				reg_write = true;
			}
			else {
				bg[1].scrollx = (bg[1].scrollx & 0x00FF) | (regs.bg2hofs << 8);
				reg_write = false;
			}
		}
			break;

		case 0x2110: {
			static bool reg_write = false;
			regs.bg2vofs = val;

			if (!reg_write) {
				bg[1].scrolly = (bg[1].scrolly & 0xFF00) | regs.bg2vofs;
				reg_write = true;
			}
			else {
				bg[1].scrolly = (bg[1].scrolly & 0x00FF) | (regs.bg2vofs << 8);
				reg_write = false;
			}
		}
			break;

		case 0x2111: {
			static bool reg_write = false;
			regs.bg3hofs = val;

			if (!reg_write) {
				bg[2].scrollx = (bg[2].scrollx & 0xFF00) | regs.bg3hofs;
				reg_write = true;
			}
			else {
				bg[2].scrollx = (bg[2].scrollx & 0x00FF) | (regs.bg3hofs << 8);
				reg_write = false;
			}
		}
			break;

		case 0x2112: {
			static bool reg_write = false;
			regs.bg3vofs = val;

			if (!reg_write) {
				bg[2].scrolly = (bg[2].scrolly & 0xFF00) | regs.bg3vofs;
				reg_write = true;
			}
			else {
				bg[2].scrolly = (bg[2].scrolly & 0x00FF) | (regs.bg3vofs << 8);
				reg_write = false;
			}
		}
			break;

		case 0x2113: {
			static bool reg_write = false;
			regs.bg4hofs = val;

			if (!reg_write) {
				bg[3].scrollx = (bg[3].scrollx & 0xFF00) | regs.bg4hofs;
				reg_write = true;
			}
			else {
				bg[3].scrollx = (bg[3].scrollx & 0x00FF) | (regs.bg4hofs << 8);
				reg_write = false;
			}
		}
			break;

		case 0x2114: {
			static bool reg_write = false;
			regs.bg4vofs = val;

			if (!reg_write) {
				bg[3].scrolly = (bg[3].scrolly & 0xFF00) | regs.bg4vofs;
				reg_write = true;
			}
			else {
				bg[3].scrolly = (bg[3].scrolly & 0x00FF) | (regs.bg4vofs << 8);
				reg_write = false;
			}
		}
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

void PPU::tick(unsigned cycles)
{
	dot += cycles;

	if (dot >= 341) {
		dot -= 341;
		scanline++;
		y = scanline - 1;
		x = 0;

		if (scanline == vblank_scanline) {
			vblank_flag = true;
			render();
		}

		if (scanline > 261) {
			dot = mclock_dot = 0;
			scanline = 0;
			x = y = 0;
			vblank_flag = false;
		}
	}

	if (scanline >= 1 && scanline <= 224 && dot >= 22 && x < 256) {
		for (int i = 0; i < cycles && x < 256; i++) {
			(this->*mr_table[regs.bgmode & 1].get_priority)();

			if (!priority_vec.size()) {
				framebuf[idx(x, y)] = { 0, 0, 0, 255 };
				x++;
				break;
			}

			(this->*mr_table[regs.bgmode & 1].render_bgp)();
		}
	}
	else if (scanline >= vblank_scanline && scanline <= 261) {
		if (vblank_flag && nmi_enable) snes->cpu.nmi_pending = true;
	}
}

void PPU::render()
{
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_RenderClear(ren);
	SDL_SetRenderTarget(ren, tex);

	SDL_FRect rect = { 0.0, 0.0, SCALE, SCALE };

	for (int y = 0; y < 224; y++) {
		for (int x = 0; x < 256; x++) {
			SDL_Color rgb = framebuf[idx(x, y)];

			SDL_SetRenderDrawColor(ren, rgb.r, rgb.g, rgb.b, rgb.a);
			SDL_RenderFillRect(ren, &rect);
		
			rect.x += SCALE;
		}

		rect.y += SCALE;
		rect.x = 0;
	}

	SDL_SetRenderTarget(ren, nullptr);
	SDL_RenderTexture(ren, tex, nullptr, nullptr);

	SDL_RenderPresent(ren);

	elapsed_tick = SDL_GetTicks() - elapsed_tick;

	if (elapsed_tick < 16) {
		SDL_Delay(16 - elapsed_tick);

		elapsed_tick = SDL_GetTicks();
	}
}
