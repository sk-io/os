
all:
	make -C api
	make -C libc
	make -C draw
	make -C terminal
	make -C files
	make -C test
	# make -C cube
	cp draw/draw.exe bin/
	cp terminal/terminal.exe bin/
	# cp cube/cube.exe bin/
	cp test/test.exe bin/
	cp files/files.exe bin/

clean:
	make -C api clean
	make -C libc clean
	make -C draw clean
	make -C terminal clean
	make -C cube clean
	make -C test clean
	make -C files clean
	rm -f bin/*.exe
