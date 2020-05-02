CFLAGS  := -std=c99 -D_GNU_SOURCE -pedantic -Wall \
	-Wdeclaration-after-statement -Wno-parentheses -I..
LIBS    := -L.. -ltermsploit

.PHONY: all
all: exploit

exploit:
	$(CC) $(CFLAGS) exploit.c -o exploit $(LIBS)

.PHONY: clean
clean:
	rm -f exploit
