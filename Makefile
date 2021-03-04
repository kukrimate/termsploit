# Compiler flags
CFLAGS  := -std=c99 -D_GNU_SOURCE -Wall -Wdeclaration-after-statement

# Library object files
LIBOBJ  := util.o termsploit.o

.PHONY: all
all: libtermsploit.a

libtermsploit.so: $(LIBOBJ)
	$(CC) $(LDFLAGS) -shared -o $@ $(LIBOBJ)

libtermsploit.a: $(LIBOBJ)
	$(AR) -cr $@ $(LIBOBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o *.a *.so
