#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
	SNES snes;
	snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\8x8BGMap4BPP32x328PAL.sfc");
	snes.run();
	return 0;
}