CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lusb-1.0

TARGETS = probe

all: $(TARGETS)

probe: probe.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

install: all
	@echo "Run with: sudo ./probe"

.PHONY: all clean install
