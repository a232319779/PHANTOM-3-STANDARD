# two elf : bk5811_demod, test
all: bk5811_demodu $(test_target)
.PHONY : all

test_target = test
test_object = $(test_target).o

CFLAGS += -Wall -std=c99
objects = main.o bk5811_demodu.o

ifeq ($(shell uname), Linux)
HACKRFCFLAGS := -I/usr/local/include/libhackrf -L/usr/local/lib -lhackrf 
endif

ifeq ($(shell uname), Darwin)
HACKRFCFLAGS := -I/opt/local/include/libhackrf -L/opt/local/lib -lhackrf
endif


# bk5811_demodu
bk5811_demodu: $(objects)
	cc -o bk5811_demodu $(objects) 

#all: $(objects)
# if the .h has been modified, should be recompile.
#$(objects):bk5811_demodu.h
$(objects):
	$(CC) -c $(CFLAGS) $< -o $@

# if the .h has been modified, should be recompile.
#bk5811_demodu.o: bk5811_demodu.h 
#main.o: bk5811_demodu.h 
#cc -c -std=c99 main.c
$(test_target) : $(test_object) 
	$(CC) -o $(test_target) $(CFLAGS) $(HACKRFCFLAGS) $(test_object)

.PHONY : clean
clean :
	rm -f bk5811_demodu $(objects) 
	rm $(test_target) $(test_object)
