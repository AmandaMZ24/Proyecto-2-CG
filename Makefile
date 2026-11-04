CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS = -lm
OBJS = main.o escena.o vectores.o poligonos.o raytracing.o framebuffer.o
BIN = raytracing

# ImageMagick 7 usa "magick", ImageMagick 6 usa "convert".
IM := $(shell command -v magick >/dev/null 2>&1 && echo magick || echo convert)
all: $(BIN)
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)
main.o: main.c escena.h framebuffer.h objetos.h
escena.o: escena.c escena.h objetos.h
vectores.o: vectores.c vectores.h objetos.h
raytracing.o: raytracing.c raytracing.h escena.h vectores.h poligonos.h objetos.h
framebuffer.o: framebuffer.c framebuffer.h raytracing.h objetos.h
poligonos.o: poligonos.c poligonos.h vectores.h objetos.h
clean:
	rm -f $(OBJS) $(BIN) pruebas/prueba_iluminacion
ESCENA ?= escena_prueba_visual.txt

run: $(BIN)
	./$(BIN) $(ESCENA) imagen.avs

jpg:
	$(IM) imagen.avs -quality 100 imagen.jpg

# Prueba del modelo de iluminacion (ambiental, difusa, especular,
# sombras y materiales). Se ejecuta desde la raiz del proyecto.
prueba_iluminacion: escena.o vectores.o poligonos.o raytracing.o
	$(CC) $(CFLAGS) -I. -o pruebas/prueba_iluminacion \
		pruebas/prueba_iluminacion.c escena.o vectores.o poligonos.o raytracing.o $(LDLIBS)
	./pruebas/prueba_iluminacion

prueba: prueba_iluminacion

.PHONY: all clean run jpg prueba prueba_iluminacion
