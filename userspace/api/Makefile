CC = clang
LD = clang
AR = ar
CFLAGS = -g -c -I include -nostdlib -ffreestanding -m32 -O2

SOURCES_C = $(patsubst %.c, %.o, $(wildcard src/*.c))
OBJ = $(SOURCES_C)

all: libos.a

libos.a: $(OBJ)
	$(AR) rcs $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f src/*.o libos.a
