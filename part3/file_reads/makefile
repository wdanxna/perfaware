CC=clang++
CFLAGS= -I ../../ -std=c++17 -O3 -Wall

all: main
main: file_reads.cpp main.cpp
	$(CC) $(CFLAGS) main.cpp -o main

clean:
	rm -rf main