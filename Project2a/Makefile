# NAME: Ryan Miyahara
# EMAIL: rmiyahara144@gmail.com
# ID: 804585999

build:
	gcc -Wall -Wextra -g -pthread lab2_add.c -o lab2_add
	gcc -Wall -Wextra -g -pthread lab2_list.c SortedList.c -o lab2_list

tests:

add:
	gcc -Wall -Wextra -g -pthread lab2_add.c -o lab2_add

graphs:

dist:
	tar -czf lab2a-804585999.tar.gz ./lab2_add.c ./SortedList.h ./SortedList.c ./lab2_list.c ./Makefile ./lab2_add.csv ./lab2_list.csv *.png ./README

clean:
	rm -f lab2_add lab2_list lab2a-804585999.tar.gz *.o