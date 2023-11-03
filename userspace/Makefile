
all:
	make -C api
	make -C libc
	mkdir -p bin
	make -C draw
	cp draw/draw.exe bin/
	make -C terminal
	cp terminal/terminal.exe bin/
	make -C files
	cp files/files.exe bin/
	make -C test
	cp test/test.exe bin/
	make -C breakout
	cp breakout/breakout.exe bin/
	# make -C cube
	# cp cube/cube.exe bin/

clean:
	make -C api clean
	make -C libc clean
	make -C draw clean
	make -C terminal clean
	make -C test clean
	make -C files clean
	make -C breakout clean
	# make -C cube clean
	rm -f bin/*.exe
