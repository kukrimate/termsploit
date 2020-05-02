CFLAGS  := -fpic -I.. `pkg-config --cflags python3`
LDFLAGS := -shared
LIBS    := -L.. -ltermsploit `pkg-config --libs python3`

.PHONY: all
all: pysploit.so

pysploit.so: pysploit.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o *.so
