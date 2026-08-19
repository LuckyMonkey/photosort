CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic

all: bin/photosweep

bin/photosweep: src/photosweep.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f bin/photosweep

.PHONY: all clean
