#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
	SNES snes(false);

	if (argc > 1) snes.load_file(argv[1]);
	else snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\snesroms\\smw.smc");

	snes.run();
	return 0;
 }