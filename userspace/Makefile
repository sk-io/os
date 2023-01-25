
all:
	make -C api
	make -C draw
	make -C terminal
	make -C test
	# make -C cube
	cp draw/draw.exe bin/
	cp terminal/terminal.exe bin/
	# cp cube/cube.exe bin/
	cp test/test.exe bin/

clean:
	make -C api clean
	make -C draw clean
	make -C terminal clean
	make -C cube clean
	make -C test clean
	rm -f bin/*.exe
