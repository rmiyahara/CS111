#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

default:
	gcc -Wall -Wextra -g -pthread lab2_list.c SortedList.c -o lab2_list

tests:
	make;
	chmod +x ./plotlist.sh;
	./plotlist.sh
profile:
	
graphs:

clean:
	rm -f ./lab2b-804585999.tar.gz ./*.png ./lab2b_list.csv ./lab2_list ./profile.out

dist:
	make clean;
	make;
	make tests;
	make profile;
	make graphs;
	tar -zcf lab2b-804585999.tar.gz ./SortedList.h ./SortedList.c ./lab2_list.c ./Makefile ./lab2b_list.csv ./profile.out ./*.png ./plotlist.sh ./README
