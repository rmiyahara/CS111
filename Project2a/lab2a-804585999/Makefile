# NAME: Ryan Miyahara
# EMAIL: rmiyahara144@gmail.com
# ID: 804585999

build:
	gcc -Wall -Wextra -g -pthread lab2_add.c -o lab2_add
	gcc -Wall -Wextra -g -pthread lab2_list.c SortedList.c -o lab2_list

ifndef VERBOSE
.SILENT:
endif

tests:
	make build;
	chmod +x plotadd.sh
	./plotadd.sh
	chmod +x plotlist.sh
	./plotlist.sh

graphs:
	chmod +x ./lab2_add.gp
	./lab2_add.gp
	chmod +x ./lab2_list.gp
	./lab2_list.gp

dist:
	make;
	make tests;
	make graphs;
	tar -czf lab2a-804585999.tar.gz ./lab2_add.c ./plotadd.sh ./SortedList.h ./SortedList.c ./lab2_list.c ./Makefile ./lab2_add.csv ./lab2_list.csv *.png ./README

clean:
	rm -f lab2_add lab2_list lab2a-804585999.tar.gz *.o *.csv *.png