#include "PPU.h"
#include "SNES.h"

uint8_t PPU::get_bpp_row(uint8_t bpp, uint16_t tmap_idx, uint16_t tset_idx, uint16_t x, uint16_t y)
{
	uint8_t ydir = (vram[tmap_idx] >> 15) & 1 ? 7 - (y % 8) : (y % 8);
	uint16_t plane_idx = tset_idx * (bpp * 4) + ydir + (tset_idx & 0xF000);
	uint8_t pal_idx = 0;

	for (int i = 0; i < bpp; i++) {
		uint8_t p = (vram[plane_idx + 8 * (i / 2)] >> 8 * is_odd(i)) & 0xFF;
		uint8_t xdir = (vram[tmap_idx] >> 14) & 1 ? 0x1 << (x % 8) : 0x80 >> (x % 8);

		bool b = (p & xdir) != 0;
		pal_idx |= (b << i);
	}

	return pal_idx;
}

uint16_t PPU::get_cgidx_by_mode(uint8_t tmap_pal, uint8_t pal_idx, uint8_t bgnum)
{
	uint16_t cgram_idx = 0;

	if (!pal_idx) return cgram_idx;

	switch (regs.bgmode & 0x7) {
		case 0: cgram_idx = 32 * bgnum + 4 * tmap_pal + pal_idx; break;
		case 1: cgram_idx = ((bgnum == 3) ? 4 : 16) * tmap_pal + pal_idx; break;
		case 3: cgram_idx = (bgnum == 1) ? 16 * tmap_pal + pal_idx : pal_idx; break;
	}

	return cgram_idx;
}

void PPU::render_scanline()
{
	for (int i = 0; i < 4; i++) {
		render_linebuf(linebuf[i], i);
	}

	copy_linebufs();
}

void PPU::render_linebuf(std::array<BufMetadata, 256>& linebuf, uint8_t bgnum)
{
	for (int x = 0; x < 256; x++) {
		uint16_t tmap_base = bg[bgnum].tilemap_base & 0x7FFF;
		uint16_t tset_base = bg[bgnum].tileset_base & 0x7FFF;

		uint16_t scrollx = (bg[bgnum].scrollx + x) % (8 * bg[bgnum].tilemap_sizex);
		uint16_t scrolly = (bg[bgnum].scrolly + y) % (8 * bg[bgnum].tilemap_sizey);

		uint16_t offset =
			(((bg[bgnum].tilemap_sizex == 64) ? (scrolly % 256) : (scrolly)) / 8) * 32 +
			((scrollx % 256) / 8) +
			(scrollx / 256) * 0x400 +
			(bg[bgnum].tilemap_sizex / 64) * ((scrolly / 256) * 0x800);

		uint16_t tmap_idx = tmap_base + offset;

		uint16_t tset_idx = tset_base + (vram[tmap_idx] & 0x3FF);

		uint8_t pal_idx = get_bpp_row(bg[bgnum].bpp, tmap_idx, tset_idx, scrollx, scrolly);

		uint8_t tmap_pal = (vram[tmap_idx] >> 10) & 0x7;
		uint16_t cgram_idx = get_cgidx_by_mode(tmap_pal, pal_idx, bg[bgnum].num);

		uint16_t entry = cgram[cgram_idx];

		if (bg[bgnum].disabled) {
			linebuf[x].priority = false;
			linebuf[x].backdrop = true;
		}
		else {
			linebuf[x].priority = (vram[tmap_idx] >> 13) & 1;
			linebuf[x].backdrop = pal_idx == 0;
		}

		linebuf[x].rgb = to_rgb888(entry);
		linebuf[x].bgnum = bgnum;
	}
}

void PPU::copy_linebufs()
{
	int last_bg = 0;

	for (int i = 0; i < 4; i++) {
		if (regs.tm & (1 << i)) last_bg = i;
	}

	switch (regs.bgmode & 0x7) {
		case 0: {
			BufMetadata result[2];
			BufMetadata tmp;

			for (int i = 0; i < 256; i++) {
				for (int j = 0; j < 4; j += 2) {
					if (linebuf[j][i].priority >= linebuf[j + 1][i].priority)
						result[j / 2] = (linebuf[j][i].backdrop && linebuf[j][i].bgnum != last_bg) ? linebuf[j + 1][i] : linebuf[j][i];
					else
						result[j / 2] = linebuf[j][i];
				}

				tmp = (result[0].backdrop && result[0].bgnum != last_bg) ? result[1] : result[0];

				framebuf[idx(i, y)] = tmp;
			}
		}
			break;

		case 1: {
			BufMetadata result;

			for (int i = 0; i < 256; i++) {
				if (linebuf[0][i].priority >= linebuf[1][i].priority)
					result = (linebuf[0][i].backdrop && linebuf[0][i].bgnum != last_bg) ? linebuf[1][i] : linebuf[0][i];
				else
					result = linebuf[0][i];

				BufMetadata bg3 = linebuf[2][i];
			
				framebuf[idx(i, y)] = (bg3.priority && regs.bgmode & 0x8 && !bg3.backdrop) ? bg3 : result;
			}
		}
			break;

		case 3: {
			BufMetadata result;

			for (int i = 0; i < 256; i++) {
				if (linebuf[0][i].priority >= linebuf[1][i].priority)
					result = linebuf[0][i];
				else
					result = linebuf[1][i];

				framebuf[idx(i, y)] = result;
			}
		}
			  break;
	}
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

	for (auto& i : linebuf)
		std::fill(i.begin(), i.end(), BufMetadata{});

	for (int i = 0; i < 4; i++) {
		bg[i].num = i;
		bg[i].tilemap_sizex = 32;
		bg[i].tilemap_sizey = 32;
	}

	stage = PRE_RENDER;

	mdr = 0;
	vram_addr = vram_inc = 0;
	cgreg_write = false;

	vblank_scanline = 224;
	vblank_flag = nmi_enable = false;
	frame_ready = false;
	dot = scanline = 0;

	y = 0;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer("SuperMosaic", 256 * SCALE, 224 * SCALE, 0, &win, &ren);
	SDL_SetRenderVSync(ren, 1);

	tex = SDL_CreateTexture(
		ren,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_TARGET,
		256 * SCALE,
		224 * SCALE
	);
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

	switch (stage) {
		case PRE_RENDER:
			if (dot >= 22) {
				stage = RENDERING;
			}
			break;

		case RENDERING:
			if (dot >= 278) {
				if (scanline >= 1 && scanline <= 224)
					render_scanline();

				stage = HBLANK;

				for (int i = 0; i < 8; i++) {
					if (snes->dma.hdma_is_enabled(i) && !snes->dma.hdma_is_terminated(i))
						snes->dma.start_hdma_transfer(i);
				}
			}
			break;

		case HBLANK:
			if (dot >= 341) {
				dot -= 341;
				scanline++;
				y = scanline - 1;

				if (scanline == 225) {
					stage = VBLANK;
					vblank_flag = frame_ready = true;
					
					snes->dma.hdma_reset();
				}
				else {
					stage = PRE_RENDER;
				}
			}
			break;

		case VBLANK:
			if (vblank_flag && nmi_enable) snes->cpu.nmi_pending = true;

			if (dot >= 341) {
				dot -= 341;
				scanline++;
				y = scanline - 1;

				if (scanline >= 261) {
					dot = 0;
					scanline = 0;
					y = 0;
					vblank_flag = false;

					stage = PRE_RENDER;
					snes->dma.hdma_reset();
				}
			}
			break;
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
			SDL_Color rgb = framebuf[idx(x, y)].rgb;

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

	frame_ready = false;
}
