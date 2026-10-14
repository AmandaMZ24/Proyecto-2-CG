/*
 * main.c
 *
 * Punto de entrada de prueba para la etapa de carga de la escena.
 * Mas adelante aqui se agregara el trazado de rayos y la escritura del .avs.
 *
 * Uso:  ./raytracing [archivo_de_escena]
 * Por defecto lee "escena.txt".
 *
 * Solo informa errores (por stderr) y una linea de resumen; no imprime
 * valores de depuracion.
 */
#include <stdio.h>
#include <stdlib.h>
#include "escena.h"
#include "framebuffer.h"

int main(int argc, char **argv) {
    const char *ruta = argc > 1 ? argv[1] : "escena.txt";
    const char *salida = argc > 2 ? argv[2] : "imagen.avs";
    Escena escena;
    if (!escena_leer(&escena, ruta)) {
        fprintf(stderr, "Error leyendo escena: %s\n", escena_error(&escena));
        escena_liberar(&escena);
        return EXIT_FAILURE;
    }
    int ok = guardar_avs(salida, &escena);
    if (!ok) fprintf(stderr, "Error guardando imagen AVS: %s\n", salida);
    escena_liberar(&escena);
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
