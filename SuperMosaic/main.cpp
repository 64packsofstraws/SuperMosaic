#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
#ifndef DEBUG
	SNES snes;
	snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\SNES-master\\PPU\\BGMAP\\8x8\\8BPP\\32x32\\8x8BGMap8BPP32x32.sfc");
	snes.run();
#else
	CPU cpu(nullptr);
	cpu.step();
#endif
	return 0;
}