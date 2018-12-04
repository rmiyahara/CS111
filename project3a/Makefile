#NAME: Alejandra Cervantes,Ryan Miyahara
#EMAIL: alecer@ucla.edu,rmiyahara144@gmail.com
#ID: 104844623,804585999

ifndef VERBOSE
.SILENT:
endif

default:
	gcc -Wall -Wextra -g lab3a.c -o lab3a

clean:
	rm -f ./lab3a-804585999.tar.gz ./lab3a

dist: default
	tar -zcf lab3a-804585999.tar.gz ./Makefile ./README ./lab3a.c ./ext2_fs.h
