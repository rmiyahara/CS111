#NAME: Ryan Miyahara
#EMAIL: rmiyahara144@gmail.com
#ID: 804585999

ifndef VERBOSE
.SILENT:
endif

default:
	gcc -Wall -Wextra -Wl,--no-as-needed -lprofiler -Wl,--as-needed -g -pthread lab2_list.c SortedList.c -o lab2_list

tests: default
	chmod +x ./plotlist.sh;
	./plotlist.sh

profile: default
	rm -f profile.out profile.gperf
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=profile.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list profile.gperf > profile.out
	pprof --list=quick_maths ./lab2_list profile.gperf >> profile.out
	rm -f ./profile.gperf
	
graphs: tests
	chmod +x lab2_list.gp;
	./lab2_list.gp

clean:
	rm -f ./lab2b-804585999.tar.gz ./lab2b_*.png ./lab2b_list.csv ./lab2_list ./profile.out

dist: profile graphs
	tar -zcf lab2b-804585999.tar.gz ./SortedList.h ./SortedList.c ./lab2_list.c ./Makefile ./lab2b_list.csv ./profile.out ./lab2b_*.png ./plotlist.sh ./lab2_list.gp ./README
