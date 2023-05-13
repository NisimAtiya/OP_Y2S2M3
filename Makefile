CC = gcc
all: ctnc


ctnc:
	CC stnc.c -o stnc


.PHONY: clean all


clean:
	rm -f ctnc large_file.txt file_received.txt