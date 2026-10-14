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
#include <unistd.h>
#include <sys/types.h>

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
    
    /* Generar y guardar la imagen AVS */
    int ok = guardar_avs(salida, &escena);

    /* Liberar la memoria utilizada por la escena */
    escena_liberar(&escena);

    if (!ok){
        fprintf(stderr, "Error guardando imagen AVS: %s\n", salida);
        return EXIT_FAILURE;
    }

    /* Mostrar la imagen generada con ImageMagick */
    pid_t pid = fork();

    if (pid < 0){
        fprintf(stderr, "Error al iniciar ImageMagick.\n");
        return EXIT_FAILURE;
    }

    if (pid == 0){
        execlp("display", "display", salida, (char *)NULL);

        fprintf(stderr, "No se pudo ejecutar ImageMagick.\n");
        _exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

