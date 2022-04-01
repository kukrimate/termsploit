# Install prefix
PREFIX := /usr/local
# Compiler flags
CFLAGS := -Wall -std=c99 -D_GNU_SOURCE
# Library object files
LIBOBJ := src/util.o src/termsploit.o src/fmtstr.o

.PHONY: all
all: libtermsploit.so libtermsploit.a

libtermsploit.so: $(LIBOBJ)
	$(CC) $(LDFLAGS) -shared -o $@ $^

libtermsploit.a: $(LIBOBJ)
	$(AR) -cr $@ $(LIBOBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: install
install: libtermsploit.so libtermsploit.a
	install -D -t $(PREFIX)/include/termsploit/ \
							src/termsploit.h src/memrw.h src/fmtstr.h
	install -D -t $(PREFIX)/lib/ libtermsploit.so libtermsploit.a
	mkdir -p $(PREFIX)/lib/pkgconfig
	sed 's|##PREFIX##|$(PREFIX)|g' misc/termsploit.pc > \
				$(PREFIX)/lib/pkgconfig/termsploit.pc

.PHONY: clean
clean:
	rm -f *.o *.a *.so
