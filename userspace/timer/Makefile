CC = clang
LD = clang
CFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -c -Wall -Wextra -I ../api/include -I ../libc/include -O2
LDFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -no-pie -L../api -los -L../libc -lc -O2
ASFLAGS = -felf32

SOURCES_C = $(patsubst %.c, %.o, $(wildcard *.c))

OBJ = $(SOURCES_C)
EXE = timer.exe

all: $(EXE)

$(EXE): $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(EXE)
