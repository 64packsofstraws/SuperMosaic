#include "PPU.h"
#include "SNES.h"

void PPU::apply_color_math(uint8_t bgnum, std::array<BufMetadata, 256>& sub_buf)
{
	for (int i = 0; i < 256; i++) {
		if (!sub_buf[i].backdrop) {
			// mask them so the alpha byte wont mess up the calculation
			uint32_t main_pix = linebuf[bgnum][i].rgb & 0xFFFFFF00;
			uint32_t sub_pix = sub_buf[i].rgb & 0xFFFFFF00;

			linebuf[bgnum][i].rgb = (regs.cgadsub & 0x80 ? main_pix - sub_pix : main_pix + sub_pix) | 0xFF;
			linebuf[bgnum][i].rgb >>= 1 * ((regs.cgadsub & 0x40) != 0);
		}
	}
}

void PPU::get_active_sprites()
{
	SpriteEntry entry;
	int sliver_count = 0;

	for (int i = 0; i < 512; i += 4) {
		entry.x = oam[i];
		entry.y = oam[i + 1];
		entry.tile_addr = oam[i + 2] * 16;
		uint8_t attr = oam[i + 3];

		uint8_t high_table = (oam[(i / 16) + 512] >> (2 * ((i / 4) % 4))) & 0x3;

		entry.x |= (high_table & 1) << 8;

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

		entry.tile_addr += base;

		entry.pal_sel = attr >> 1 & 0x7;
		entry.priority = attr >> 4 & 0x3;
		entry.hflip = attr & 0x40;
		entry.vflip = attr & 0x80;

		if (entry.x > 256 && entry.x + entry.width - 1 < 512) continue;

		if (y >= entry.y && y < entry.y + entry.height) {
			active_sprites.push_back(entry);
			sliver_count += entry.width / 8;

			if (sliver_count >= 34) regs.stat77 |= 0x80;

			if (active_sprites.size() == 32) {
				regs.stat77 |= 0x40;
				break;
			}
		}
	}
}

void PPU::render_sprites()
{
	for (auto& entry : active_sprites) {
		int tile_per_row = entry.width / 8;

		for (int x = entry.x, i = 0; x < 256 && i < tile_per_row; i++) {
			uint8_t ydir = entry.vflip ? (entry.height - 1) - (y - entry.y) : y - entry.y;

			uint16_t sprite_idx =
				entry.tile_addr + 
				ydir +
				((ydir / 8) * 0xF8) +
				(entry.hflip ? (tile_per_row - 1 - i) : i) * 16;

			uint8_t p1 = vram[sprite_idx] & 0xFF;
			uint8_t p2 = vram[sprite_idx] >> 8;
			uint8_t p3 = vram[sprite_idx + 8] & 0xFF;
			uint8_t p4 = vram[sprite_idx + 8] >> 8;

			for (int j = 0; j < 8; j++) {
				uint8_t xdir = entry.hflip ? 0x1 << j : 0x80 >> j;
				bool b1 = (p1 & xdir) != 0;
				bool b2 = (p2 & xdir) != 0;
				bool b3 = (p3 & xdir) != 0;
				bool b4 = (p4 & xdir) != 0;

				uint8_t pal_idx = (b4 << 3) | (b3 << 2) | (b2 << 1) | b1;

				if (pal_idx) {
					linebuf[3 - entry.priority][x].rgb = to_rgb888(cgram[16 * (8 + entry.pal_sel) + pal_idx]);
					linebuf[3 - entry.priority][x].backdrop = false;
					linebuf[3 - entry.priority][x].priority = true;
				}
				x++;

				if (x > 255) break;
			}
		}
	}

	active_sprites.clear();
}

uint8_t PPU::get_bpp_row(uint8_t bpp, uint16_t tset_base, uint16_t tmap_idx, uint16_t tset_idx, uint16_t x, uint16_t y)
{
	uint8_t ydir = (vram[tmap_idx] >> 15) & 1 ? 7 - (y % 8) : (y % 8);
	uint8_t xdir = (vram[tmap_idx] >> 14) & 1 ? 0x1 << (x % 8) : 0x80 >> (x % 8);

	uint16_t plane_idx = (tset_idx * (bpp * 4) + ydir + tset_base) & 0x7FFF;
	uint8_t pal_idx = 0;

	auto is_odd = [&](int n) { return n % 2; };

	for (int i = 0; i < bpp; i++) {
		uint8_t p = (vram[plane_idx + 8 * (i / 2)] >> (8 * is_odd(i))) & 0xFF;

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

	std::array<BufMetadata, 256> sub_buf = mix_linebufs(sub_last_bg);

	uint8_t main_last_bg = 0;

	for (int i = 0; i < 4; i++) {
		if (!bg[i].in_main) {
			BufMetadata tmp = { to_rgb888(cgram[0]), false, true, i };
			std::fill(linebuf[i].begin(), linebuf[i].end(), tmp);
			continue;
		}

		main_last_bg = i;
		render_linebuf(linebuf[i], i);

		if (regs.cgadsub & (1 << i) && regs.cgwsel & 0x2) apply_color_math(i, sub_buf);
	}

	get_active_sprites();

	if (!active_sprites.empty() || regs.tm & 0x10) render_sprites();

	std::array<BufMetadata, 256> main_buf = mix_linebufs(main_last_bg);

	for (int x = 0; x < 256; x++) {
		if (!main_buf[x].backdrop) {
			framebuf[idx(x, y)] = main_buf[x].rgb;
		}
		else if (!sub_buf[x].backdrop) {
			framebuf[idx(x, y)] = sub_buf[x].rgb;
		}
		else {
			framebuf[idx(x, y)] = (!(regs.cgwsel & 0x10) && (regs.cgadsub & 0x20)) ?
				to_rgb888((fc.b << 10) | (fc.g << 5) | fc.r) : to_rgb888(cgram[0]);
		}
	}
}

void PPU::render_linebuf(std::array<BufMetadata, 256>& linebuf, uint8_t bgnum)
{
	uint8_t mosaic = 1;
	if (regs.mosaic & (1 << bgnum)) mosaic = ((regs.mosaic >> 4) & 0xF) + 1;

	for (int x = 0; x < 256; x++) {
		uint16_t tmap_base = bg[bgnum].tilemap_base & 0x7FFF;
		uint16_t tset_base = bg[bgnum].tileset_base & 0x7FFF;

		uint16_t scrollx = (bg[bgnum].scrollx + (x - (x % mosaic))) % (8 * bg[bgnum].tilemap_sizex);
		uint16_t scrolly = (bg[bgnum].scrolly + (y - (y % mosaic))) % (8 * bg[bgnum].tilemap_sizey);

		uint16_t offset =
			(((bg[bgnum].tilemap_sizex == 64) ? (scrolly % 256) : (scrolly)) / 8) * 32 +
			((scrollx % 256) / 8) +
			(scrollx / 256) * 1024 +
			(bg[bgnum].tilemap_sizex / 64) * ((scrolly / 256) * 2048);

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

std::array<PPU::BufMetadata, 256> PPU::mix_linebufs(uint8_t last_bg)
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
		printf("BG mode %d not supported\n", regs.bgmode & 0x7);
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