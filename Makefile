CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic
LDLIBS = -lm
MOTOR_OBJS = escena.o vectores.o poligonos.o discos.o elipses.o cilindros.o conos.o cortes.o ecuaciones.o cuadraticas.o texturas.o antialiasing.o raytracing.o
OBJS = main.o $(MOTOR_OBJS) framebuffer.o
BIN = raytracing

# ImageMagick 7 usa "magick", ImageMagick 6 usa "convert".
IM := $(shell command -v magick >/dev/null 2>&1 && echo magick || echo convert)
all: $(BIN)
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)
main.o: main.c escena.h framebuffer.h objetos.h
escena.o: escena.c escena.h objetos.h vectores.h poligonos.h texturas.h
vectores.o: vectores.c vectores.h objetos.h
texturas.o: texturas.c texturas.h vectores.h objetos.h
antialiasing.o: antialiasing.c antialiasing.h raytracing.h objetos.h
raytracing.o: raytracing.c raytracing.h escena.h vectores.h poligonos.h discos.h elipses.h cilindros.h conos.h cortes.h ecuaciones.h cuadraticas.h texturas.h antialiasing.h objetos.h
framebuffer.o: framebuffer.c framebuffer.h raytracing.h objetos.h
poligonos.o: poligonos.c poligonos.h vectores.h objetos.h
discos.o: discos.c discos.h vectores.h objetos.h poligonos.h
cilindros.o: cilindros.c cilindros.h vectores.h objetos.h poligonos.h
conos.o: conos.c conos.h vectores.h objetos.h poligonos.h
elipses.o: elipses.c elipses.h vectores.h objetos.h poligonos.h
cortes.o: cortes.c cortes.h vectores.h objetos.h
ecuaciones.o: ecuaciones.c ecuaciones.h
cuadraticas.o: cuadraticas.c cuadraticas.h vectores.h objetos.h poligonos.h
clean:
	rm -f $(OBJS) $(BIN) pruebas/prueba_iluminacion pruebas/prueba_opticos
ESCENA ?= escena.txt

run: $(BIN)
	./$(BIN) $(ESCENA) imagen.avs

jpg:
	$(IM) imagen.avs -quality 100 imagen.jpg

# Renderiza todas las escenas de prueba (escenas_prueba/*.txt) y las
# convierte a PNG en salida_pruebas/ para revisarlas rapidamente.
# SIN_DISPLAY evita abrir ImageMagick por cada escena.
prueba_escenas: $(BIN)
	@mkdir -p salida_pruebas
	@for f in escenas_prueba/*.txt; do \
		n=$$(basename $$f .txt); \
		echo "== $$n =="; \
		if ! SIN_DISPLAY=1 ./$(BIN) $$f salida_pruebas/$$n.avs \
			2> salida_pruebas/$$n.log; then \
			cat salida_pruebas/$$n.log; exit 1; \
		fi; \
		$(IM) salida_pruebas/$$n.avs salida_pruebas/$$n.png; \
	done
	@echo "Imagenes en salida_pruebas/"

prueba_iluminacion: $(MOTOR_OBJS)
	$(CC) $(CFLAGS) -I. -o pruebas/prueba_iluminacion \
		pruebas/prueba_iluminacion.c $(MOTOR_OBJS) $(LDLIBS)
	./pruebas/prueba_iluminacion

prueba_opticos: $(MOTOR_OBJS)
	$(CC) $(CFLAGS) -I. -o pruebas/prueba_opticos \
		pruebas/prueba_opticos.c $(MOTOR_OBJS) $(LDLIBS)
	./pruebas/prueba_opticos

prueba: prueba_iluminacion prueba_opticos

.PHONY: all clean run jpg prueba_escenas prueba prueba_iluminacion prueba_opticos
