
all:
	make -C api
	make -C draw
	make -C terminal
	# make -C cube
	cp draw/draw.exe bin/
	cp terminal/terminal.exe bin/
	# cp cube/cube.exe bin/

clean:
	make -C api clean
	make -C draw clean
	make -C terminal clean
	make -C cube clean
	rm -f bin/*.exe
