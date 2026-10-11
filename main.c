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

int main(int argc, char **argv)
{
    const char *ruta = (argc > 1) ? argv[1] : "escena.txt";
    Escena escena;
    int ok;

    ok = escena_leer(&escena, ruta);
    if (!ok) {
        fprintf(stderr, "Error en %s\n", escena_error(&escena));
        escena_liberar(&escena);
        return EXIT_FAILURE;
    }

    printf("Escena \"%s\" cargada: %d objetos (%d esferas), "
           "framebuffer %dx%d\n",
           ruta, escena.num_objetos, escena_num_esferas(&escena),
           escena.ancho, escena.alto);

    escena_liberar(&escena);
    return EXIT_SUCCESS;
}
