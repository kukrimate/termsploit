.PHONY: all
all:
	cc -DSPAWN exploit.c ../libtermsploit.a -o exploit_spawn
	cc exploit.c ../libtermsploit.a -o exploit_connect

.PHONY: clean
clean:
	rm -f exploit_spawn exploit_connect
