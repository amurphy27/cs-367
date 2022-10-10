CC=gcc -Wall

all: main_read_stdin read_stdin trie

main_read_stdin: main_read_stdin.c read_stdin
	$(CC) -o main_read_stdin main_read_stdin.c read_stdin.c

read_stdin: read_stdin.c
	$(CC) -c read_stdin.c

trie: trie.c trie.h
	$(CC) -c trie.c

clean:
	rm -f main_read_stdin read_stdin.o trie.o
