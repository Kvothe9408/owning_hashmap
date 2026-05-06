hashmap: hashmap.c
	gcc -Wall -Wextra -fsanitize=address -g hashmap.c -o hashmap

clean:
	rm -f hashmap hashmap.o

.PHONY: clean
