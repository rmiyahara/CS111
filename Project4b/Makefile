#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

ifndef VERBOSE
.SILENT:
endif

default:
	gcc -Wall -Wextra -lmraa -lm -g lab4b.c -o lab4b

check: default
	chmod +x check.sh
	./check.sh

clean:
	rm -f ./lab4b-804585999.tar.gz ./lab4b

dist: default
	tar -zcf lab24b-804585999.tar.gz ./Makefile ./README ./lab4b.c ./check.sh