CC = g++
CFLAGS = -Wall -std=c++11
SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:%.c=%.o)

.PHONY: all clean
all: dragonshell
clean:
	rm *.o dragonshell
%.o: %.cc
	${CC} ${CFLAGS} -c $< -o $@

dragonshell: $(OBJECTS)
	$(CC) $(CFLAGS) -o dragonshell $(OBJECTS)
