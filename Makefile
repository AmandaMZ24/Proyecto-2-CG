CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS = -lm

OBJS = main.o escena.o
BIN  = raytracing

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

main.o: main.c escena.h objetos.h
	$(CC) $(CFLAGS) -c main.c -o main.o

escena.o: escena.c escena.h objetos.h
	$(CC) $(CFLAGS) -c escena.c -o escena.o

clean:
	rm -f $(OBJS) $(BIN)

.PHONY: all clean
