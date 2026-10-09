CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = 1_sigint 2_alarm 3_signal_block

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
