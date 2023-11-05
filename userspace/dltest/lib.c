#include "lib.h"

// __attribute__((visibility("hidden")))
int foo;

int YEPPERS() {
	return 42;
}

int frick(int nono) {
	return 666 + nono - foo;
}

void helpme() {
	foo = 9 + YEPPERS();
}
