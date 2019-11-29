CC=gcc
CFLAGS = -Wall -Werror
all: filesystem
clean:
	rm *.o fs
compress: fs-sim.tar.gz
fs-sim.tar.gz:
	cd .. && tar -cvzf fs-sim.tar.gz fs-sim/
filesystem: fs
fs: FileSystem.o
	${CC} ${CFLAGS} -o fs FileSystem.o 
compile: FileSystem.o 
FileSystem.o: FileSystem.c
	${CC} ${CFLAGS} -c FileSystem.c
