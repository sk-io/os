CC = i686-elf-gcc
LD = i686-elf-gcc
CFLAGS = -g -mgeneral-regs-only -std=gnu11 -nostdlib -ffreestanding -m32 -c -Wall -Wextra -lgcc -I ../api/include -O2
LDFLAGS = -g -mgeneral-regs-only -std=gnu11 -nostdlib -ffreestanding -lgcc -L../api -los -O2
ASFLAGS = -felf32

SOURCES_C = $(patsubst %.c, %.o, $(wildcard *.c))

OBJ = $(SOURCES_C)
EXE = cube.exe

all: $(OBJ) link

link: $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
