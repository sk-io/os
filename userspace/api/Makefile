CC = clang
LD = clang
CFLAGS = -g -c -I include -nostdlib -fvisibility=hidden -ffreestanding -m32 -fpic
LDFLAGS = -g -ffreestanding -nostdlib -fvisibility=hidden -m32 -shared -fpic

SOURCES_C = $(patsubst %.c, %.o, $(wildcard src/*.c))
OBJ = $(SOURCES_C)

all: api.so crt0.o

api.so: $(OBJ)
	$(LD) -o $@ $(OBJ) $(LDFLAGS)

crt0.o: crt0/crt0.c
	$(CC) -g -c -I include -nostdlib -ffreestanding -m32 $< -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f src/*.o api.so crt0.o
