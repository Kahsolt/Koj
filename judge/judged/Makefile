.PHONY: all test clean

all: clean
	rm -rf build && mkdir build && cd build && cmake .. && make || exit 1

test:
	cd example && make test
	cd test    && make test

clean:
	cd example && make clean