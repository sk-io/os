
all:
	make -C api
	make -C libc
	mkdir -p bin
	make -C draw
	cp draw/*.exe bin/
	make -C terminal
	cp terminal/*.exe bin/
	make -C files
	cp files/*.exe bin/
	make -C timer
	cp timer/*.exe bin/
	make -C breakout
	cp breakout/*.exe bin/
	make -C test
	cp test/*.exe bin/
	make -C dltest
	cp dltest/*.exe bin/
	cp dltest/*.so bin/

clean:
	make -C api clean
	make -C libc clean
	make -C draw clean
	make -C terminal clean
	make -C files clean
	make -C breakout clean
	make -C timer clean
	make -C test clean
	# make -C dltest clean
	# make -C cube clean
	rm -f bin/*.exe bin/*.so
