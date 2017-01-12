CFLAGS += -Wall -std=c99
objects = main.o bk5811_demodu.o

# 2 elf bk5811 and test
all: bk5811_demodu test
.PHONY : all

# bk5811
bk5811_demodu: $(objects)
	cc -o bk5811_demodu $(objects) 

all: $(objects)
# if the .h has been modified, should be recompile.
$(objects):bk5811_demodu.h
$(objects):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

# if the .h has been modified, should be recompile.
#bk5811_demodu.o: bk5811_demodu.h 
#main.o: bk5811_demodu.h 
#cc -c -std=c99 main.c

# test
CFLAGS += -I/usr/local/include/libhackrf -L/usr/local/lib -lhackrf 
test.o:
test: test.o 
	cc $(CFLAGS) -o test test.o

.PHONY : clean
clean :
	rm -f bk5811_demodu $(objects) 
	rm -f test.o test
