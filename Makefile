# NOTE: Feel free to change the makefile to suit your own need.

# compile and link flags
CC=g++

all: 
	g++ -std=c++0x -pthread dsdv.cpp rip.cpp socket.cpp -o dsdv


clean:
	rm -f *~ *.o dsdv
