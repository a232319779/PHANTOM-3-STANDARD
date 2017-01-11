compile:
	gcc -std=c99 -g -o decode_bk5811 bk5811_demodu.c

.PHONY : clean
	rm decode_bk5811
