CFLAGS  := `pkg-config --cflags termsploit` \
			-std=c99 -D_GNU_SOURCE -Wall -Wdeclaration-after-statement
LDLIBS  := `pkg-config --libs termsploit`

.PHONY: all
all: exploit

exploit: exploit.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	rm -f exploit
