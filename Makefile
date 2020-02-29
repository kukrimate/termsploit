CFLAGS := -std=c99 -D_GNU_SOURCE
OBJ    := termsploit.o example.o

termsploit: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f *.o termsploit
