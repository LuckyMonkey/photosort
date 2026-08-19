CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic

all: bin/photosweep

bin/photosweep: src/photosweep.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $<

check: bin/photosweep
	@test "$$(./bin/photosweep 2>&1 | head -1)" = "usage: photosweep run TYPE ROOT OUT"
	rm -rf /tmp/photosweep-demo-check
	./bin/photosweep run all archive/legacy-photosort/images /tmp/photosweep-demo-check
	@test "$$(wc -l < /tmp/photosweep-demo-check/ocr.jsonl)" -eq 3
	@test "$$(wc -l < /tmp/photosweep-demo-check/faces.jsonl)" -eq 3
	@test "$$(wc -l < /tmp/photosweep-demo-check/gps.jsonl)" -eq 3
	@test "$$(wc -l < /tmp/photosweep-demo-check/swatch.jsonl)" -eq 3

clean:
	rm -f bin/photosweep

.PHONY: all check clean
