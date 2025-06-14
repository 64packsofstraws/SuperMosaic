#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
#ifndef DEBUG
	SNES snes;
	snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\SNES-master\\PPU\\BGMAP\\8x8\\2BPP\\8x8BG4Map2BPP32x328PAL\\8x8BG4Map2BPP32x328PAL.sfc");
	snes.run();
#else
	CPU cpu(nullptr);
	cpu.step();
#endif
	return 0;
}