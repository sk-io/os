CC = clang
AR = ar
LIB = libsgfx.a
CFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -Wall -Wextra -lgcc -O2

all: $(LIB)

$(LIB): sgfx.o
	$(AR) rcs $@ $<

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o $(LIB)
