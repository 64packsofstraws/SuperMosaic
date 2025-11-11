#include <iostream>
#include "SNES.h"

int main(int argc, char** argv)
{
	SNES snes(false);
	snes.load_file("C:\\Users\\Aaron Straw\\Downloads\\ctrl.smc");
	snes.run();
	return 0;
 }