/*
 * main.c
 *
 * Punto de entrada: lee la escena, traza la imagen, guarda el .avs
 * y la muestra con ImageMagick (display).
 *
 * Uso:  ./raytracing [archivo_de_escena] [salida.avs]
 * Por defecto lee "escena.txt" y escribe "imagen.avs".
 *
 * Solo informa errores (por stderr) y una linea de resumen; no imprime
 * valores de depuracion.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "escena.h"
#include "framebuffer.h"
#include "bvh.h"

/*
 * Decide si debe abrirse ImageMagick: solo si hay un entorno grafico
 * (X11 o Wayland) y no se pide lo contrario con SIN_DISPLAY=1.
 */
static int debe_mostrar(void)
{
    const char *sin = getenv("SIN_DISPLAY");

    if (sin != NULL && sin[0] != '\0' && strcmp(sin, "0") != 0) {
        return 0;
    }

    return getenv("DISPLAY") != NULL || getenv("WAYLAND_DISPLAY") != NULL;
}

int main(int argc, char **argv) {
    const char *ruta = argc > 1 ? argv[1] : "escena.txt";
    const char *salida = argc > 2 ? argv[2] : "imagen.avs";
    Escena escena = {0};
    if (!escena_leer(&escena, ruta)) {
        fprintf(stderr, "Error leyendo escena: %s\n", escena_error(&escena));
        escena_liberar(&escena);
        return EXIT_FAILURE;
    }

    escena.bvh = bvh_construir(&escena);
    
    /* Generar y guardar la imagen AVS */
    int ok = guardar_avs(salida, &escena);

    /* Liberar la memoria utilizada por la escena */
    escena_liberar(&escena);

    if (!ok){
        fprintf(stderr, "Error guardando imagen AVS: %s\n", salida);
        return EXIT_FAILURE;
    }

    /* Mostrar la imagen generada con ImageMagick */
    if (debe_mostrar()) {
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

        /*
         * El padre no espera: al terminar, el proceso de ImageMagick
         * queda re-parentado al init y no se convierte en zombi.
         */
    }

    return EXIT_SUCCESS;
}

