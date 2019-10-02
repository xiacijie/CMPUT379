CC = gcc
CFLAGS = -Wall
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:%.c=%.o)

.PHONY: all clean
all: dragonshell
clean:
	rm *.o dragonshell
%.o: %.cc
	${CC} ${CFLAGS} -c $< -o $@

dragonshell: $(OBJECTS)
	$(CC) $(CFLAGS) -o dragonshell $(OBJECTS)
