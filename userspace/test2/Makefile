CC = i686-elf-gcc
LD = i686-elf-gcc
CFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -c -Wall -Wextra -lgcc -I ../api/include -I ../libc/include -O2
LDFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -lgcc -L../api -L../libc -los -lc -O2
ASFLAGS = -felf32

SOURCES_C = $(patsubst %.c, %.o, $(wildcard *.c))

OBJ = $(SOURCES_C)
EXE = test2.exe

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
