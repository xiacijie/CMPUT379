CC=g++
CFLAGS = -Wall -Werror -std=c++11 -pthread
all: wc
clean:
	rm *.o wordcount
compress: mapreduce.tar.gz
mapreduce.tar.gz:
	cd .. && tar -cvzf mapreduce.tar.gz mapreduce/
wc: wordcount
wordcount: distwc.o threadpool.o mapreduce.o datastructure.o
	${CC} ${CFLAGS} -o wordcount distwc.o threadpool.o mapreduce.o datastructure.o
compile: distwc.o threadpool.o mapreduce.o datastructure.o
distwc.o: distwc.cc
	${CC} ${CFLAGS} -c distwc.cc
threadpool.o: threadpool.cc
	${CC} ${CFLAGS} -c threadpool.cc
mapreduce.o: mapreduce.cc
	${CC} ${CFLAGS} -c mapreduce.cc
datastructure.o: datastructure.cc
	${CC} ${CFLAGS} -c datastructure.cc
	
