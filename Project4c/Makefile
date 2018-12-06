#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

ifndef VERBOSE
.SILENT:
endif

default:
	gcc -Wall -Wextra -lmraa -lm -g lab4c_tcp.c -o lab4c_tcp
	gcc -Wall -Wextra -lmraa -lm -lssl -lcrypto -g lab4c_tls.c -o lab4c_tls

tcp:
	gcc -Wall -Wextra -lmraa -lm -g lab4c_tcp.c -o lab4c_tcp

tls:
	gcc -Wall -Wextra -lmraa -lm -lssl -lcrypto -g lab4c_tls.c -o lab4c_tls

clean:
	rm -f ./lab4c-804585999.tar.gz ./lab4c_tcp ./lab4c_tls ./*.txt

dist: default
	tar -zcf lab4c-804585999.tar.gz ./Makefile ./README ./lab4c_tcp.c ./lab4c_tls.c