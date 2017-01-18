# two elf : bk5811_demod, test
all: bk5811_demodu test real_time_decode
.PHONY : all

CFLAGS += -Wall -std=c99
objects = main.o bk5811_demodu.o
test_objects = test.o
real_time_objects = real_time_decode.o

ifeq ($(shell uname), Linux)
HACKRF_INCLUDE := -I/usr/local/include/libhackrf
HACKRF_LIB := -L/usr/local/lib -lhackrf 
endif

ifeq ($(shell uname), Darwin)
HACKRF_INCLUDE := -I/usr/local/include/libhackrf
HACKRF_LIB := -L/usr/local/lib -lhackrf
endif

# build bk5811_demodu
# if the .h has been modified, should be recompile.
$(objects):bk5811_demodu.h
$(objects) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
# bk5811_demodu
bk5811_demodu: $(objects)
	$(CC) -o bk5811_demodu $(objects) 

# build test
$(test_objects) : test.h common.h 
$(test_objects) : %.o : %.c
	$(CC) -c $(CFLAGS) $(HACKRF_INCLUDE) $< -o $@ 
# test
test : $(test_objects) 
	$(CC) -o test $(CFLAGS) $(HACKRF_LIB) $(test_objects)

# build real_time_decode
$(real_time_objects) : common.h
$(real_time_objects) : %.o : %.c
	$(CC) -c $(CFLAGS) $(HACKRF_INCLUDE) $< -o $@
# real_time_decode
real_time_decode : $(real_time_objects)
	$(CC) -o real_time_decode $(CFLAGS) $(HACKRF_LIB) $(real_time_objects)

.PHONY : clean
clean :
	-rm -f bk5811_demodu $(objects) 
	-rm -f test $(test_objects)
	-rm -f real_time_decode $(real_time_objects)
