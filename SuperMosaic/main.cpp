#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
	SNES snes;
	snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\smw.smc");
	snes.run();
	return 0;
 }