CFLAGS  := -std=c99 -D_GNU_SOURCE -Wall -Wdeclaration-after-statement -I..
LDLIBS  := -L.. -ltermsploit

.PHONY: all
all: exploit

exploit: exploit.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	rm -f exploit
