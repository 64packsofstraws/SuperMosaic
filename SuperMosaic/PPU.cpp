#include "PPU.h"
#include "SNES.h"

void PPU::get_active_sprites()
{
	SpriteEntry entry;

	for (int i = 0; i < 512; i += 4) {
		entry.x = oam[i];
		entry.y = oam[i + 1];
		entry.tile_addr = oam[i + 2];
		uint8_t attr = oam[i + 3];

		uint8_t high_table = (oam[(i / 16) + 512] >> 2 * ((i / 4) % 4)) & 0x3;

		entry.x |= high_table & 1;

		switch ((regs.objsel >> 5) & 0x7) {
			case 0:
				entry.height = entry.width = high_table & 0x2 ? 16 : 8;
				break;

			case 1:
				entry.height = entry.width = high_table & 0x2 ? 32 : 8;
				break;

			case 2:
				entry.height = entry.width = high_table & 0x2 ? 64 : 8;
				break;

			case 3:
				entry.height = entry.width = high_table & 0x2 ? 32 : 16;
				break;

			case 4:
				entry.height = entry.width = high_table & 0x2 ? 64 : 16;
				break;

			case 5:
				entry.height = entry.width = high_table & 0x2 ? 64 : 32;
				break;

			case 6:
				if (high_table & 0x2) {
					entry.width = 16;
					entry.height = 32;
				}
				else {
					entry.width = 32;
					entry.height = 64;
				}
				break;

			case 7:
				if (high_table & 0x2) {
					entry.width = 16;
					entry.height = 32;
				}
				else {
					entry.width = 32;
					entry.height = 32;
				}
				break;
		}
		
		uint16_t base = attr & 1 ? sprite_page1 : sprite_page0;

		entry.tile_addr |= base;

		entry.pal_sel = attr >> 1 & 0x7;
		entry.priority = attr >> 4 & 0x3;
		entry.hflip = attr & 0x40;
		entry.vflip = attr & 0x80;

		if (entry.x > 256 && entry.x + entry.width - 1 < 512) continue;

		if (y >= entry.y && y < entry.y + entry.height) {
			active_sprites.push_back(entry);

			if (active_sprites.size() == 32) {
				regs.stat77 |= 0x40;
				break;
			}
		}
	}
}

uint8_t PPU::get_bpp_row(uint8_t bpp, uint16_t tset_base, uint16_t tmap_idx, uint16_t tset_idx, uint16_t x, uint16_t y)
{
	uint8_t ydir = (vram[tmap_idx] >> 15) & 1 ? 7 - (y % 8) : (y % 8);
	uint16_t plane_idx = (tset_idx * (bpp * 4) + ydir + tset_base) & 0x7FFF;
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
		case 1: cgram_idx = ((bgnum == 2) ? 4 : 16) * tmap_pal + pal_idx; break;
		case 3: cgram_idx = (bgnum == 1) ? 16 * tmap_pal + pal_idx : pal_idx; break;
	}

	return cgram_idx;
}

void PPU::render_scanline()
{
	if (regs.inidisp & 0x80) {
		std::fill(framebuf.begin() + idx(0, y), framebuf.begin() + idx(255, y), 0);
		return;
	}

	uint8_t main_last_bg = 0;

	for (int i = 0; i < 4; i++) {
		if (!bg[i].in_main) {
			BufMetadata tmp = { to_rgb888(cgram[0]), false, true, i };
			std::fill(linebuf[i].begin(), linebuf[i].end(), tmp);
			continue;
		}

		main_last_bg = i;
		render_linebuf(linebuf[i], i);
	}

	std::array<BufMetadata, 256> main_buf = copy_linebufs(main_last_bg);
	
	uint8_t sub_last_bg = 0;
	for (int i = 0; i < 4; i++) {
		if (!bg[i].in_sub) {
			BufMetadata tmp = { 0, false, true, i };
			std::fill(linebuf[i].begin(), linebuf[i].end(), tmp);
			continue;
		}

		sub_last_bg = i;
		render_linebuf(linebuf[i], i);
	}

	std::array<BufMetadata, 256> sub_buf = copy_linebufs(sub_last_bg);

	for (int i = 0; i < 256; i++) {
		if (!main_buf[i].backdrop) {
			framebuf[idx(i, y)] = main_buf[i].rgb;
		}
		else if (!sub_buf[i].backdrop) {
			framebuf[idx(i, y)] = sub_buf[i].rgb;
		}
		else {
			framebuf[idx(i, y)] = to_rgb888(cgram[0]);
		}
	}
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

		uint8_t pal_idx = get_bpp_row(bg[bgnum].bpp, tset_base, tmap_idx, tset_idx, scrollx, scrolly);

		uint8_t tmap_pal = (vram[tmap_idx] >> 10) & 0x7;
		uint16_t cgram_idx = get_cgidx_by_mode(tmap_pal, pal_idx, bg[bgnum].num);

		uint16_t entry = cgram[cgram_idx];

		linebuf[x].priority = (vram[tmap_idx] >> 13) & 1;
		linebuf[x].backdrop = pal_idx == 0;
		linebuf[x].rgb = to_rgb888(entry);
		linebuf[x].bgnum = bgnum;
	}
}

std::array<PPU::BufMetadata, 256> PPU::copy_linebufs(uint8_t last_bg)
{
	std::array<BufMetadata, 256> buf;

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

				buf[i] = tmp;
			}
		}
			break;

		case 1: {
			BufMetadata result;

			for (int i = 0; i < 256; i++) {
				if (linebuf[0][i].priority >= linebuf[1][i].priority)
					result = (linebuf[0][i].backdrop && linebuf[0][i].bgnum != last_bg) ? linebuf[1][i] : linebuf[0][i];
				else
					result = linebuf[1][i];

				BufMetadata bg3 = linebuf[2][i];
			
				buf[i] = (bg3.priority && (regs.bgmode & 0x8) && !bg3.backdrop) ? bg3 : result;
			}
		}
			break;

		case 3: {
			BufMetadata result;

			for (int i = 0; i < 256; i++) {
				if (linebuf[0][i].priority >= linebuf[1][i].priority)
					result = (linebuf[0][i].backdrop && linebuf[0][i].bgnum != last_bg) ? linebuf[1][i] : linebuf[0][i];
				else
					result = linebuf[1][i];

				buf[i] = result;
			}
		}
			  break;

		default:
			printf("BG mode %d not supported\n", regs.bgmode);
			exit(1);
	}

	return buf;
}

uint32_t PPU::to_rgb888(uint16_t rgb)
{
	uint8_t r = rgb & 0x1F;
	uint8_t g = (rgb >> 5) & 0x1F;
	uint8_t b = (rgb >> 10) & 0x1F;

	float brightness = static_cast<float>(regs.inidisp & 0xF) / 15.0f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	r = static_cast<uint8_t>(r * brightness);
	g = static_cast<uint8_t>(g * brightness);
	b = static_cast<uint8_t>(b * brightness);

	return (r << 24) | (g << 16) | (b << 8) | 0xFF;
}

PPU::PPU(SNES* snes) : snes(snes), vram(0x8000, 0), cgram(256, 0), framebuf(256 * 224, 0), oam(544, 0)
{
	memset(&regs, 0, sizeof(regs));
	memset(&bg, 0, sizeof(bg));

	for (int i = 0; i < 4; i++) {
		bg[i].num = i;
		bg[i].tilemap_sizex = 32;
		bg[i].tilemap_sizey = 32;
		bg[i].in_main = false;
		bg[i].in_sub = false;
	}

	stage = PRE_RENDER;

	mdr = 0;
	vram_addr = vram_inc = 0;
	cgreg_write = false;
	counter_latch = false;
	m7_latch = 0;
	opvct_byte = ophct_byte = false;

	internal_oamadd = oam_latch = 0;

	vblank_scanline = 224;
	vblank_flag = nmi_enable = false;
	interlace_frame = false;
	frame_ready = false;
	dot = scanline = 0;

	y = 0;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer("SuperMosaic", 256 * SCALE, 224 * SCALE, 0, &win, &ren);
	SDL_SetRenderVSync(ren, 2);

	tex = SDL_CreateTexture(
		ren,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		256,
		224
	);

	SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
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
	for (int i = 0; i < cycles; i++) {
		uint8_t nmititen = snes->bus.regs.nmitimen;
		uint16_t htime = snes->bus.regs.htime;
		uint16_t vtime = snes->bus.regs.vtime;

		switch ((nmititen >> 4) & 0x3) {
			case 1:
				if (dot == htime) {
					snes->cpu.irq_pending = true;
					snes->bus.regs.timeup |= 0x80;
				}
				else {
					snes->cpu.irq_pending = false;
					snes->bus.regs.timeup &= ~0x80;
				}
				break;

			case 2:
				if (scanline == vtime && dot == 0) {
					snes->cpu.irq_pending = true;
					snes->bus.regs.timeup |= 0x80;
				}
				else {
					snes->cpu.irq_pending = false;
					snes->bus.regs.timeup &= ~0x80;
				}
				break;

			case 3:
				if (scanline == vtime && dot == htime) {
					snes->cpu.irq_pending = true;
					snes->bus.regs.timeup |= 0x80;
				}
				else {
					snes->cpu.irq_pending = false;
					snes->bus.regs.timeup &= ~0x80;
				}
				break;
		}

		dot++;
	}


	switch (stage) {
		case PRE_RENDER:
			if (dot >= 22) {
				stage = RENDERING;
			}
			break;

		case RENDERING:
			if (dot >= 278) {
				if (scanline >= 1 && scanline <= 224) {
					render_scanline();

					if (scanline == 224) frame_ready = true;
				}

				snes->bus.regs.hvbjoy |= 0x40;
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

				if (scanline == vblank_scanline) {
					stage = VBLANK;
					vblank_flag = true;
					snes->bus.regs.hvbjoy |= 0x80;
					snes->bus.regs.hvbjoy &= ~0x40;
					snes->joypad.update_autoread();

					snes->dma.hdma_reset();
				}
				else {
					snes->bus.regs.hvbjoy &= ~0x40;
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
					y = scanline - 1;
					snes->bus.regs.rdnmi &= 0x7F;
					snes->bus.regs.hvbjoy &= ~0x80;

					regs.stat77 &= ~0x40;
					
					vblank_flag = false;
					interlace_frame = !interlace_frame;

					stage = PRE_RENDER;
					snes->dma.hdma_reset();
				}
			}
			break;
	}
}

void PPU::render()
{
	SDL_UpdateTexture(tex, NULL, framebuf.data(), 256 * sizeof(uint32_t));
	SDL_RenderClear(ren);
	SDL_RenderTexture(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);

	frame_ready = false;
}
