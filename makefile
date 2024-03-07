smallsh: smallsh.c parser.c signal_handler.c
	gcc -std=c99 -o smallsh smallsh.c parser.c signal_handler.c

clean:
	rm -f smallsh