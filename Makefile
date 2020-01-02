override CFLAGS=-Wall -Werror

all:	clean
all:
	$(CC) $(CFLAGS) lsusers.c -o lsusers


O2:	CFLAGS += -O2
O2:	all


debug:	CFLAGS += -g -Og
debug:	all

clean:
	rm -f lsusers
