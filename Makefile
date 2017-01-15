test_target = test
test_object = $(test_target).o

# two elf : bk5811_demod, test
all: bk5811_demodu $(test_target)
.PHONY : all

CFLAGS += -Wall -std=c99
objects = main.o bk5811_demodu.o

ifeq ($(shell uname), Linux)
HACKRF_INCLUDE := -I/usr/local/include/libhackrf
HACKRF_LIB := -L/usr/local/lib -lhackrf 
endif

ifeq ($(shell uname), Darwin)
HACKRF_INCLUDE := -I/opt/local/include/libhackrf
HACKRF_LIB := -L/opt/local/lib -lhackrf
endif


# bk5811_demodu
bk5811_demodu: $(objects)
	cc -o bk5811_demodu $(objects) 

# if the .h has been modified, should be recompile.
$(objects):bk5811_demodu.h
main.o : main.c
bk5811_demodu.o : bk5811_demodu.c

$(test_object) : $(test_target).c
	$(CC) -o $(test_object) -c $(test_target).c $(CFLAGS) $(HACKRF_INCLUDE) 
$(test_target) : $(test_object) 
	$(CC) -o $(test_target) $(CFLAGS) $(HACKRF_INCLUDE) $(HACKRF_LIB) $(test_object)

.PHONY : clean
clean :
	-rm -f bk5811_demodu $(objects) 
	-rm $(test_target) $(test_object)
