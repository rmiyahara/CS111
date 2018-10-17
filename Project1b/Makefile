#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

default:
	gcc -Wall -Wextra -g lab1b-client.c -o lab1b-client;
	gcc -Wall -Wextra -g lab1b-server.c -o lab1b-server

client:
	gcc -Wall -Wextra -g lab1b-client.c -o lab1b-client

server:
	gcc -Wall -Wextra -g lab1b-server.c -o lab1b-server

clean:
	rm -f lab1b *.o *tar.gz *.txt

dist:
	make;
	tar -czf lab1b-804585999.tar.gz ./lab1b-client.c ./lab1b-server.c ./Makefile ./my.key ./README