#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <SDL3/SDL.h>

#define idx(x, y) (y * 256 + x)
constexpr int SCALE = 2;

class SNES;

class PPU
{
	std::vector<uint16_t> vram;
	std::vector<uint16_t> cgram;
	std::vector<uint8_t> oam;

	struct SpriteEntry {
		uint16_t x;
		uint8_t y;
		uint16_t tile_addr;
		uint8_t pal_sel;
		bool vflip;
		bool hflip;
		uint8_t priority;
		uint8_t width;
		uint8_t height;
	};

	uint16_t sprite_page0;
	uint16_t sprite_page1;

	std::vector<SpriteEntry> active_sprites;

	struct BufMetadata {
		uint32_t rgb;
		bool priority;
		bool backdrop;
		uint8_t bgnum;
	};

	enum Stages {
		PRE_RENDER,
		RENDERING,
		HBLANK,
		VBLANK
	} stage;

	std::vector<uint32_t> framebuf;
	uint8_t y;

	std::array<std::array<BufMetadata, 256>, 4> linebuf;

	struct Background {
		uint8_t num;
		uint8_t bpp;
		uint16_t tilemap_base;
		uint16_t tileset_base;
		uint8_t tilemap_sizex;
		uint8_t tilemap_sizey;
		uint16_t scrollx;
		uint16_t scrolly;
		bool in_main;
		bool in_sub;
	} bg[4];

	struct Regs {
		uint8_t inidisp;
		uint8_t objsel;
		uint8_t oamadd;
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
		uint16_t m7b;
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
		uint16_t ophct;
		uint16_t opvct;
		uint8_t oamdataread;
		uint8_t vmdatalread;
		uint8_t vmdatahread;
		uint16_t cgdataread;
		uint8_t stat77;
		uint8_t stat78;
	} regs;

	struct FixedColorChannels {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} fc;

	uint16_t vram_addr;
	uint16_t vram_latch;
	unsigned vram_inc;
	bool cgreg_write;
	bool counter_latch;
	bool ophct_byte;
	bool opvct_byte;
	uint8_t m7_latch;

	uint16_t internal_oamadd;
	uint8_t oam_latch;

	uint8_t mdr;

	uint8_t vblank_scanline;
	bool vblank_flag;
	bool nmi_enable;
	bool interlace_frame;

	unsigned dot;
	unsigned scanline;

	SNES* snes;

	void apply_color_math(uint8_t bgnum, std::array<BufMetadata, 256>& sub_buf);

	void get_active_sprites();
	void render_sprites();

	uint8_t get_bpp_row(uint8_t bpp, uint16_t tmap_base, uint16_t tmap_idx, uint16_t tset_idx, uint16_t x, uint16_t y);
	uint16_t get_cgidx_by_mode(uint8_t tmap_pal, uint8_t pal_idx, uint8_t bgnum);

	void render_scanline();
	void render_linebuf(std::array<BufMetadata, 256>& linebuf, uint8_t bgnum);

	std::array<PPU::BufMetadata, 256> mix_linebufs(uint8_t last_bg);

	uint32_t to_rgb888(uint16_t rgb);
public:
	SDL_Window* win;
	SDL_Renderer* ren;
	SDL_Texture* tex;
	bool frame_ready;

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

