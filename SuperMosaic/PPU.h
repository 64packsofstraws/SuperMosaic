#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <SDL3/SDL.h>

#define idx(x, y) y * 256 + x
#define is_even(n) (!(n % 2))
#define SCALE 2

class SNES;

class PPU
{
	std::vector<uint16_t> vram;
	std::vector<uint16_t> cgram;

	std::vector<SDL_Color> framebuf;
	int x;
	int y;

	struct Background {
		uint8_t num;
		uint16_t tilemap_base;
		uint16_t tileset_base;
		uint8_t tilemap_sizex;
		uint8_t tilemap_sizey;
		uint16_t scrollx;
		uint16_t scrolly;
	} bg[4];

	std::vector<Background> priority_vec;

	struct Regs {
		uint8_t inidisp;
		uint8_t objsel;
		uint8_t oamaddl;
		uint8_t oamaddh;
		uint8_t oamdata;
		uint8_t bgmode;
		uint8_t mosaic;
		uint8_t bg1sc;
		uint8_t bg2sc;
		uint8_t bg3sc;
		uint8_t bg4sc;
		uint8_t bg12nba;
		uint8_t bg34nba;
		uint8_t bg1hofs;
		uint8_t bg1vofs;
		uint8_t bg2hofs;
		uint8_t bg2vofs;
		uint8_t bg3hofs;
		uint8_t bg3vofs;
		uint8_t bg4hofs;
		uint8_t bg4vofs;
		uint8_t vmain;
		uint8_t vmaddl;
		uint8_t vmaddh;
		uint8_t vmdatal;
		uint8_t vmdatah;
		uint8_t m7sel;
		uint16_t m7a;
		uint8_t m7b;
		uint8_t m7c;
		uint8_t m7d;
		uint8_t m7x;
		uint8_t m7y;
		uint8_t cgadd;
		uint8_t cgdata;
		uint8_t w12sel;
		uint8_t w34sel;
		uint8_t wobjsel;
		uint8_t wh0;
		uint8_t wh1;
		uint8_t wh2;
		uint8_t wh3;
		uint8_t wbglog;
		uint8_t wobjlog;
		uint8_t tm;
		uint8_t ts;
		uint8_t tmw;
		uint8_t tsw;
		uint8_t cgwsel;
		uint8_t cgadsub;
		uint8_t coldata;
		uint8_t setini;
		uint8_t mpyl;
		uint8_t mpym;
		uint8_t mpyh;
		uint8_t slhv;
		uint8_t oamdataread;
		uint8_t vmdatalread;
		uint8_t vmdatahread;
		uint16_t cgdataread;
	} regs;

	uint8_t mode;

	uint16_t vram_addr;
	unsigned vram_inc;
	bool cgreg_write;

	uint8_t mdr;

	uint8_t vblank_scanline;
	bool vblank_flag;
	bool nmi_enable;

	int dot;
	int mclock_dot;
	int scanline;

	SNES* snes;

	void get_priority_m0();
	void render_bgpixel_m0();

	void get_priority_m1();
	void render_bgpixel_m1();

	void get_priority_m3();
	void render_bgpixel_m3();

	uint8_t get_2bpp_row(uint16_t tset_idx);
	uint8_t get_4bpp_row(uint16_t tset_idx);
	uint8_t get_8bpp_row(uint16_t tset_idx);

	struct ModeRender {
		void (PPU::*get_priority)();
		void (PPU::*render_bgp)();
	};

	ModeRender mr_table[4];

	SDL_Color to_rgb888(uint16_t rgb);
public:
	SDL_Window* win;
	SDL_Renderer* ren;
	SDL_Texture* tex;
	Uint64 elapsed_tick;

	PPU(SNES* snes);
	~PPU();
	
	bool get_vblank_flag() const;
	void set_vblank_flag(bool val);

	void set_nmi_enable(bool val);

	uint8_t read_reg(uint16_t addr);
	void write_reg(uint16_t addr, uint8_t val);

	void tick(unsigned mclock);
	void render();
};

