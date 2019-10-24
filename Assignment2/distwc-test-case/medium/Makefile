CC=g++
CFLAGS = -Wall -Werror -std=c++11 -pthread
all: distwc
clean-result: 
	rm -rf result-*.txt
clean-all:
	rm *.o distwc
	rm -rf result-*.txt
distwc: distwc.o threadpool.o mapreduce.o 
	${CC} ${CFLAGS} -o distwc distwc.o threadpool.o mapreduce.o 
compile: distwc.o threadpool.o mapreduce.o 
distwc.o: distwc.cc
	${CC} ${CFLAGS} -c distwc.cc
threadpool.o: threadpool.cc
	${CC} ${CFLAGS} -c threadpool.cc
mapreduce.o: mapreduce.cc
	${CC} ${CFLAGS} -c mapreduce.cc

	

