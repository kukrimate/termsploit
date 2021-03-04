CFLAGS  := `pkg-config --cflags termsploit` \
			`pkg-config --cflags python3` -fpic
LDFLAGS := -shared
LDLIBS  := `pkg-config --libs termsploit` \
		   `pkg-config --libs python3`

.PHONY: all
all: pysploit.so

pysploit.so: pysploit.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o *.so
