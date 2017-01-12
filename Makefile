CFLAGS += -std=c99
objects = main.o bk5811_demodu.o

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

.PHONY : clean
clean :
	rm -f bk5811_demodu $(objects) 
