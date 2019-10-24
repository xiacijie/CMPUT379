CC=g++
CFLAGS = -Wall -Werror -std=c++11 -pthread
TARGET=distwc

all: threadpool.o mapreduce.o
clean-all:
	rm *.o
threadpool.o: threadpool.cc
	${CC} ${CFLAGS} -c threadpool.cc
mapreduce.o: mapreduce.cc
	${CC} ${CFLAGS} -c mapreduce.cc

	
