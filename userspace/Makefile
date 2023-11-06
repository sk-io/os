CC = clang
LD = clang
CFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -c -Wall -Wextra -I ../api/include -I ../libc/include -O2
LDFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -no-pie -L../api -l:api.so -L../libc -lc -O2 ../api/crt0.o
ASFLAGS = -felf32

export

all:
	mkdir -p bin
	$(MAKE) -C api
	cp api/*.so bin/
	$(MAKE) -C libc
	$(MAKE) -C files
	cp files/*.exe bin/
	$(MAKE) -C terminal
	cp terminal/*.exe bin/
	$(MAKE) -C draw
	cp draw/*.exe bin/
	$(MAKE) -C timer
	cp timer/*.exe bin/
	$(MAKE) -C breakout
	cp breakout/*.exe bin/
	$(MAKE) -C test
	cp test/*.exe bin/

clean:
	$(MAKE) -C api clean
	$(MAKE) -C libc clean
	$(MAKE) -C draw clean
	$(MAKE) -C terminal clean
	$(MAKE) -C files clean
	$(MAKE) -C breakout clean
	$(MAKE) -C timer clean
	$(MAKE) -C test clean
	rm -f bin/*.exe bin/*.so
