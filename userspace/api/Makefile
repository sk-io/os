CC = clang
LD = clang
CFLAGS = -g -c -I include -nostdlib -fvisibility=hidden -ffreestanding -m32 -O2 -fpic
LDFLAGS = -g -ffreestanding -nostdlib -fvisibility=hidden -m32 -shared -fpic

SOURCES_C = $(patsubst %.c, %.o, $(wildcard src/*.c))
OBJ = $(SOURCES_C)

all: api.so

api.so: $(OBJ)
	$(LD) -o $@ $(OBJ) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f src/*.o api.so
