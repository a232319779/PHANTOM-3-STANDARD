bk5811_demodu: main.o bk5811_demodu.o
	cc -o bk5811_demodu main.o bk5811_demodu.o
bk5811_demodu.o: bk5811_demodu.c 
	cc -c -std=c99 bk5811_demodu.c
main.o: main.c bk5811_demodu.h
	cc -c -std=c99 main.c

.PHONY : clean
clean :
	rm -f bk5811_demodu *.o 
