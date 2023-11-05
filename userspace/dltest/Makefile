
OBJ = $(SOURCES_C)
EXE = dltest.exe

all:
	clang -g -ffreestanding -nostdlib -fvisibility=hidden -m32 -shared -fpic -o lib.so lib.c
	clang -g -no-pie -ffreestanding -nostdlib -m32 -o $(EXE) dltest.c -L. -l:lib.so

clean:
	rm -f *.o *.exe *.so
