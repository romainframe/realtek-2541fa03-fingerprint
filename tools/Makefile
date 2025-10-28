CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lusb-1.0

TARGETS = probe probe_advanced probe_control

all: $(TARGETS)

probe: probe.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

probe_advanced: probe_advanced.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

probe_control: probe_control.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)

install: all
	@echo "Run with: sudo ./probe or sudo ./probe_advanced"

.PHONY: all clean install
