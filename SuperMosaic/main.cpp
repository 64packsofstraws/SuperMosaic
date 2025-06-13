#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
#ifndef DEBUG
	SNES snes;
	snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\bg1map.sfc");
	snes.run();
#else
	CPU cpu(nullptr);
	cpu.step();
#endif
	return 0;
}