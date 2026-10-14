CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS = -lm
OBJS = main.o escena.o vectores.o raytracing.o framebuffer.o
BIN = raytracing
all: $(BIN)
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)
main.o: main.c escena.h framebuffer.h objetos.h
escena.o: escena.c escena.h objetos.h
vectores.o: vectores.c vectores.h objetos.h
raytracing.o: raytracing.c raytracing.h vectores.h objetos.h
framebuffer.o: framebuffer.c framebuffer.h raytracing.h objetos.h
clean:
	rm -f $(OBJS) $(BIN)
run: $(BIN)
	./$(BIN) escena.txt imagen.avs

jpg:
	magick imagen.avs -quality 100 imagen.jpg

.PHONY: all clean run jpg
