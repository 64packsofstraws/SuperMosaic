#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
	SNES snes;
	snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\SNES-master\\PPU\\BGMAP\\8x8\\8BPP\\64x64\\8x8BGMap8BPP64x64.sfc");
	snes.run();
	return 0;
 }