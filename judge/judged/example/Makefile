SRC         = main.c
BIN         = main

.PHONY: all clean test test_ffi

all:
	gcc -g -Wall -ansi ${SRC} -o ${BIN}
	./${BIN} < 1.in > 1.out

clean:
	rm -f ${BIN} 1.out 1.inout

test:
	make
	make test_ffi
	make clean

test_ffi:
	ruby test.rb
	# ruby testmq.rb && sleep 1

test_daemon:
	../bin/judged -h && ../bin/judged -t
	(../bin/judged &) && sleep 1
	ruby testd.rb
	(cat koj.pid | xargs kill -9) && rm -f koj.pid
