compile:
	gcc -std=c99 -g -o decode_bk5811 bk5811_demodu.c
	gcc -std=c99 -g -o  test test.c -I /usr/include/libhackrf -L /usr/lib/arm-linux-gnueabihf

.PHONY : clean
clean:
	rm decode_bk5811 test
