.PHONY: all install clean

CC = gcc -Wall -O0

all: bin/themer

install: bin/themer
	cp bin/themer ~/scripts/bin/

clean:
	rm bin/*.o

bin/themer: bin/themer.o bin/logger.o bin/parser.o
	$(CC) -Wall -o bin/themer bin/themer.o bin/logger.o bin/parser.o

bin/logger.o: header/logger.h source/logger.c
	$(CC) -Wall -o bin/logger.o -I header/ source/logger.c -c

bin/parser.o: header/parser.h source/parser.c
	$(CC) -Wall -o bin/parser.o -I header/ source/parser.c -c

bin/themer.o: source/themer.c
	$(CC) -Wall -o bin/themer.o -I header/ source/themer.c -c
