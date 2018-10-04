#Name: Ryan Miyahara
#Email: rmiyahara144@gmail.com
#ID: 804585999

default:
	gcc -Wall -Wextra -g lab0.c -o lab0

clean:
	rm -f lab0 *.o *.txt *tar.gz

dist:
	make;
	tar -czf lab0-804585999.tar.gz ./lab0.c ./Makefile ./README ./backtrace.png ./breakpoint.png

ifndef VERBOSE
.SILENT:
endif
catchsegfault-test:
	./lab0 -sc &> /dev/null; \
	if [[ $$? -ne 4 ]]; then \
		echo "catchsegfault-test failed" >> testoutput.txt; \
	fi

normal-test:
	echo "Testerino" > normal-test-input.txt; \
	./lab0 --input normal-test-input.txt --output normal-test-output.txt;
	if [[ $$? -ne 0 ]]; then \
		echo "normal-test failed" >> testoutput.txt; \
	fi; \
	rm normal-test-input.txt normal-test-output.txt

noinput-test:
	./lab0 --input noinput-test.txt --output noinput-test-output.txt &> /dev/null; \
	if [[ $$? -ne 2 ]]; then \
		echo "noinput-test failed" >> testoutput.txt; \
	fi

badoutput-test:
	touch badoutput.txt; \
	chmod -w badoutput.txt; \
	echo "Testerino" > badoutput-input.txt; \
	./lab0 --input badoutput-input.txt --output badoutput.txt &> /dev/null; \
	if [[ $$? -ne 3 ]]; then \
		echo "badoutput-test failed" >> testoutput.txt; \
	fi; \
	chmod +w badoutput.txt; \
	rm badoutput.txt badoutput-input.txt

catchsegfaultextend-test:
	echo "Testerino" > catchsegfaultextend-input.txt; \
	./lab0 --input catchsegfaultextend-input.txt --output catchsegfaultextend-output.txt --catch --segfault &> /dev/null; \
	if [[ $$? -ne 4 ]]; then \
		echo "catchsegfaultextend-test failed" >> testoutput.txt; \
	fi; \
	rm catchsegfaultextend-input.txt catchsegfaultextend-output.txt

badargument-test:
	./lab0 --yoink &> /dev/null; \
	if [[ $$? -ne 1 ]]; then \
		echo "badargument-test failed" >> testoutput.txt; \
	fi; \

check:
	touch testoutput.txt; \
	make; \
	make catchsegfault-test; \
	make normal-test; \
	make noinput-test; \
	make badoutput-test; \
	make catchsegfaultextend-test; \
	touch empty.txt; \
	cmp empty.txt testoutput.txt; \
	make VERBOSE=1; \
	if [[ $$? -eq 0 ]]; then \
		echo "Passed all the tests!"; \
	else \
		cat testoutput.txt; \
	fi; \
	rm testoutput.txt empty.txt