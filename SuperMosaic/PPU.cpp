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

		uint8_t pal_idx = get_2bpp_row(tset_idx);

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

	if (regs.tm & 0x4) {
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

		uint8_t pal_idx = (i->num == 3) ? get_2bpp_row(tset_idx) : get_4bpp_row(tset_idx);

		if (!pal_idx && i != priority_vec.end() - 1) continue;

		uint8_t tmap_pal = (vram[tmap_idx] >> 10) & 0x7;
		uint16_t entry = (!pal_idx) ? cgram[0] : cgram[((i->num == 3) ? 4 : 16) * tmap_pal + pal_idx];

		framebuf[idx(x, y)] = to_rgb888(entry);
		x++;
		break;
	}

	priority_vec.clear();
}

void PPU::get_priority_m3()
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
}

void PPU::render_bgpixel_m3()
{
	for (auto i = priority_vec.begin(); i != priority_vec.end(); i++) {
		uint16_t tmap_base = i->tilemap_base & 0x7FFF;
		uint16_t tset_base = i->tileset_base & 0x7FFF;

		uint16_t tmap_idx = tmap_base + ((y / 8) * i->tilemap_sizex + (x / 8));
		uint16_t tset_idx = tset_base + (vram[tmap_idx] & 0x3FF);

		uint8_t pal_idx = (i->num == 0) ? get_8bpp_row(tset_idx) : get_4bpp_row(tset_idx);

		if (!pal_idx && i != priority_vec.end() - 1) continue;

		uint8_t tmap_pal = (vram[tmap_idx] >> 10) & 0x7;

		uint8_t cgram_idx;
		if (i->num == 1) {
			cgram_idx = (!pal_idx) ? 0 : 16 * tmap_pal + pal_idx;
		}
		else {
			cgram_idx = pal_idx;
		}

		uint16_t entry = cgram[cgram_idx];
		framebuf[idx(x, y)] = to_rgb888(entry);
		x++;
		break;
	}

	priority_vec.clear();
}

uint8_t PPU::get_2bpp_row(uint16_t tset_idx)
{
	uint16_t plane_idx = tset_idx * 8 + (y % 8) + (tset_idx & 0xF000);

	uint8_t p0 = vram[plane_idx] & 0xFF;
	uint8_t p1 = (vram[plane_idx] >> 8) & 0xFF;

	bool b0 = ( p0 & ( 0x80 >> (x % 8) )) != 0;
	bool b1 = ( p1 & ( 0x80 >> (x % 8) )) != 0;

	uint8_t pal_idx = (b1 << 1) | b0;
	return pal_idx;
}

uint8_t PPU::get_4bpp_row(uint16_t tset_idx)
{
	uint16_t plane_idx = tset_idx * 16 + (y % 8) + (tset_idx & 0xF000);

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

	return pal_idx;
}

uint8_t PPU::get_8bpp_row(uint16_t tset_idx)
{
	uint16_t plane_idx = tset_idx * 32 + (y % 8) + (tset_idx & 0xF000);

	uint8_t p0 = vram[plane_idx] & 0xFF;
	uint8_t p1 = (vram[plane_idx] >> 8) & 0xFF;
	uint8_t p2 = vram[plane_idx + 8] & 0xFF;
	uint8_t p3 = (vram[plane_idx + 8] >> 8) & 0xFF;
	uint8_t p4 = vram[plane_idx + 16] & 0xFF;
	uint8_t p5 = (vram[plane_idx + 16] >> 8) & 0xFF;
	uint8_t p6 = vram[plane_idx + 24] & 0xFF;
	uint8_t p7 = (vram[plane_idx + 24] >> 8) & 0xFF;

	bool b0 = ( p0 & ( 0x80 >> (x % 8) )) != 0;
	bool b1 = ( p1 & ( 0x80 >> (x % 8) )) != 0;
	bool b2 = ( p2 & ( 0x80 >> (x % 8) )) != 0;
	bool b3 = ( p3 & ( 0x80 >> (x % 8) )) != 0;
	bool b4 = ( p4 & ( 0x80 >> (x % 8) )) != 0;
	bool b5 = ( p5 & ( 0x80 >> (x % 8) )) != 0;
	bool b6 = ( p6 & ( 0x80 >> (x % 8) )) != 0;
	bool b7 = ( p7 & ( 0x80 >> (x % 8) )) != 0;

	uint8_t pal_idx = (b1 << 1) | b0;
	pal_idx |= (b3 << 3) | (b2 << 2);
	pal_idx |= (b5 << 5) | (b4 << 4);
	pal_idx |= (b7 << 7) | (b6 << 6);

	return pal_idx;
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
	mr_table[3].get_priority = &PPU::get_priority_m3;
	mr_table[3].render_bgp = &PPU::render_bgpixel_m3;

	mdr = 0;
	vram_addr = vram_inc = 0;
	cgreg_write = false;

	vblank_scanline = 224;
	vblank_flag = nmi_enable = false;
	dot = scanline = mclock_dot = 0;

	x = y = 0;

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
}

PPU::~PPU()
{
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
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

void PPU::tick(unsigned cycles)
{
	dot += cycles;

	if (dot >= 341) {
		dot -= 341;
		scanline++;
		y = scanline - 1;
		x = 0;

		if (scanline == 224) {
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
			(this->*mr_table[regs.bgmode & 0x3].get_priority)();

			if (!priority_vec.size()) {
				framebuf[idx(x, y)] = { 0, 0, 0, 255 };
				x++;
				break;
			}

			(this->*mr_table[regs.bgmode & 0x3].render_bgp)();
		}
	}
	else if (scanline >= 224 && scanline <= 261) {
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
