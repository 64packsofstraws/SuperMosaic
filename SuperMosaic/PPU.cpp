#include "PPU.h"
#include "SNES.h"

PPU::PPU(SNES* snes) : snes(snes), vram(0x8000, 0), cgram(256, 0), framebuf(256 * 224, 0), oam(544, 0)
{
	memset(&regs, 0, sizeof(regs));
	memset(&bg, 0, sizeof(bg));
	memset(&fc, 0, sizeof(fc));

	for (int i = 0; i < 4; i++) {
		bg[i].num = i;
		bg[i].tilemap_sizex = 32;
		bg[i].tilemap_sizey = 32;
		bg[i].in_main = false;
		bg[i].in_sub = false;
	}

	stage = PRE_RENDER;

	mdr = 0;
	vram_addr = vram_inc = vram_latch = 0;
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
				break;

			case 2:
				if (scanline == vtime && dot == 0) {
					snes->cpu.irq_pending = true;
					snes->bus.regs.timeup |= 0x80;
				}
				break;

			case 3:
				if (scanline == vtime && dot == htime) {
					snes->cpu.irq_pending = true;
					snes->bus.regs.timeup |= 0x80;
				}
				break;
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

						if (snes->bus.regs.nmitimen & 1) {
							snes->joypad.update_autoread();
							snes->bus.regs.hvbjoy |= 0x1;
						}

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

					if (scanline == vblank_scanline + 3)
						snes->bus.regs.hvbjoy &= ~0x1;

					if (scanline >= 261) {
						dot = 0;
						scanline = 0;
						y = scanline - 1;
						snes->bus.regs.rdnmi &= 0x7F;
						snes->bus.regs.hvbjoy &= ~0x80;

						regs.stat77 &= ~0x40;
						regs.stat77 &= ~0x80;
						
						vblank_flag = false;
						interlace_frame = !interlace_frame;

						stage = PRE_RENDER;
						snes->dma.hdma_reset();
					}
				}
				break;
		}
		
		dot++;
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
