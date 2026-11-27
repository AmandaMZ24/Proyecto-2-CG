#include "framebuffer.h"
#include "raytracing.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

/* Tiempo real transcurrido, en segundos (C11). */
static double tiempo_actual(void){
    struct timespec t;
    if (timespec_get(&t, TIME_UTC) != TIME_UTC) return 0.0;
    return (double)t.tv_sec + (double)t.tv_nsec / 1000000000.0;
}

/*
 * Convierte un canal de color del rango [0, 1]
 * al rango [0, 255] utilizado por AVS.
 */
static unsigned char a_byte(double x){
    if (!(x > 0.0))return 0;
    if (x >= 1.0) return 255;
    return (unsigned char)(x * 255.0 + 0.5);
}


/*
 * Genera una imagen AVS a partir de la escena.
 * Devuelve 1 si se guarda correctamente y 0 si ocurre un error.
 */
int guardar_avs(const char *ruta, const Escena *escena){
    if (ruta == NULL || escena == NULL || escena->ancho <= 0 || escena->alto <= 0){
        return 0;
    }
    FILE *archivo = fopen(ruta, "wb");
    if (archivo == NULL){
        return 0;
    }

    /* Dimensiones de la imagen */
    uint32_t ancho = (uint32_t)escena->ancho;
    uint32_t alto = (uint32_t)escena->alto;

    /* Cabecera AVS: ancho y alto en orden big-endian */
    unsigned char cabecera[8] = {
        (unsigned char)(ancho >> 24),
        (unsigned char)(ancho >> 16),
        (unsigned char)(ancho >> 8),
        (unsigned char)ancho,

        (unsigned char)(alto >> 24),
        (unsigned char)(alto >> 16),
        (unsigned char)(alto >> 8),
        (unsigned char)alto
    };

    int correcto = 1;

    /* Escribir cabecera */
    if (fwrite(cabecera, 1, sizeof(cabecera), archivo)!= sizeof(cabecera)){
        correcto = 0;
    }

    /* Barra de avance: filas terminadas, tiempo y estimacion restante. */
    double inicio = tiempo_actual();
    int ultimo_porcentaje = -1;
    fprintf(stderr, "Renderizando %ux%u...\n", ancho, alto);
    fflush(stderr);

    /* Generar y escribir cada pixel */
    /* Reservar memoria para la imagen completa */
size_t cantidad_bytes =
    (size_t)ancho * alto * 4;

unsigned char *imagen = correcto
    ? malloc(cantidad_bytes)
    : NULL;

if (imagen == NULL) {

    correcto = 0;

} else {

    int filas_terminadas = 0;

    /* Renderizado paralelo por filas */
    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < escena->alto; y++) {

        for (int x = 0; x < escena->ancho; x++) {

            Color color = trazar_pixel(
                escena,
                x,
                y
            );

            size_t indice =
                ((size_t)y * ancho + x) * 4;

            imagen[indice] = 255;

            imagen[indice + 1] =
                a_byte(color.r);

            imagen[indice + 2] =
                a_byte(color.g);

            imagen[indice + 3] =
                a_byte(color.b);
        }

        /* Actualizar el progreso de forma segura */
        #pragma omp critical
        {
            filas_terminadas++;

            int porcentaje = (int)(
                100.0 * filas_terminadas /
                escena->alto
            );

            if (porcentaje != ultimo_porcentaje) {

                double transcurrido =
                    tiempo_actual() - inicio;

                double restante =
                    transcurrido *
                    (escena->alto - filas_terminadas) /
                    filas_terminadas;

                fprintf(
                    stderr,
                    "\rAvance: %3d%% | %d/%d filas | "
                    "Transcurrido: %.0f s | "
                    "Restante aprox.: %.0f s   ",
                    porcentaje,
                    filas_terminadas,
                    escena->alto,
                    transcurrido,
                    restante
                );

                fflush(stderr);

                ultimo_porcentaje = porcentaje;
            }
        }
    }

    /* Guardar los pixeles en el orden correcto */
    if (fwrite(
            imagen,
            1,
            cantidad_bytes,
            archivo
        ) != cantidad_bytes) {

        correcto = 0;
    }

    free(imagen);
}

    /* Comprobar que el archivo se cierre correctamente */
    if (fclose(archivo) != 0){
        correcto = 0;
    }

    /* Eliminar el archivo si la escritura fallo */
    if (!correcto){
        remove(ruta);
    }

    return correcto;
}

