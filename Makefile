all:
	gcc src/client/main.c -std=c99 -Wall -Wextra -o client
	gcc src/server/main.c -std=c99 -Wall -Wextra -o server
client:
	gcc src/client/main.c -std=c99 -Wall -Wextra -o client
	./client
server:
	gcc src/server/main.c -std=c99 -Wall -Wextra -o server
	./server
