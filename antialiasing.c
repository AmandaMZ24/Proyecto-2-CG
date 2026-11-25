#include <math.h>

#include "antialiasing.h"
#include "raytracing.h"

#define UMBRAL_COLOR 0.10
#define PROFUNDIDAD_AA 2

/* Distancia euclidiana en el espacio RGB */
double distancia_colores(
    Color a,
    Color b
)
{
    double dr = a.r - b.r;
    double dg = a.g - b.g;
    double db = a.b - b.b;

    return sqrt(
        dr * dr +
        dg * dg +
        db * db
    );
}

/* Promedio de cuatro colores */
static Color promedio_colores(
    Color a,
    Color b,
    Color c,
    Color d
)
{
    return (Color){

        (a.r + b.r + c.r + d.r) / 4.0,

        (a.g + b.g + c.g + d.g) / 4.0,

        (a.b + b.b + c.b + d.b) / 4.0
    };
}

/* Verificar si las cuatro muestras son similares */
static int colores_similares(
    Color a,
    Color b,
    Color c,
    Color d
)
{
    Color colores[4] = {a, b, c, d};

    for (int i = 0; i < 4; i++) {

        for (int j = i + 1; j < 4; j++) {

            if (distancia_colores(
                    colores[i],
                    colores[j]
                ) > UMBRAL_COLOR) {

                return 0;
            }
        }
    }

    return 1;
}

/* Evaluar una region del pixel */
static Color muestreo_adaptativo(
    const Escena *escena,
    double x0,
    double y0,
    double x1,
    double y1,
    Color a,
    Color b,
    Color c,
    Color d,
    int profundidad
)
{
    /* Condiciones de parada */
    if (profundidad >= PROFUNDIDAD_AA ||
        colores_similares(a, b, c, d)) {

        return promedio_colores(
            a, b, c, d
        );
    }

    /* Coordenadas centrales */
    double xm = (x0 + x1) / 2.0;
    double ym = (y0 + y1) / 2.0;

    /*
     * Obtener solamente las cinco
     * muestras nuevas.
     */

    Color arriba = trazar_muestra(
        escena, xm, y0
    );

    Color abajo = trazar_muestra(
        escena, xm, y1
    );

    Color izquierda = trazar_muestra(
        escena, x0, ym
    );

    Color derecha = trazar_muestra(
        escena, x1, ym
    );

    Color centro = trazar_muestra(
        escena, xm, ym
    );

    /* Cuadrante superior izquierdo */
    Color q1 = muestreo_adaptativo(
        escena,
        x0, y0,
        xm, ym,
        a, arriba,
        izquierda, centro,
        profundidad + 1
    );

    /* Cuadrante superior derecho */
    Color q2 = muestreo_adaptativo(
        escena,
        xm, y0,
        x1, ym,
        arriba, b,
        centro, derecha,
        profundidad + 1
    );

    /* Cuadrante inferior izquierdo */
    Color q3 = muestreo_adaptativo(
        escena,
        x0, ym,
        xm, y1,
        izquierda, centro,
        c, abajo,
        profundidad + 1
    );

    /* Cuadrante inferior derecho */
    Color q4 = muestreo_adaptativo(
        escena,
        xm, ym,
        x1, y1,
        centro, derecha,
        abajo, d,
        profundidad + 1
    );

    /* Integrar las cuatro regiones */
    return promedio_colores(
        q1, q2, q3, q4
    );
}

Color antialiasing_pixel(
    const Escena *escena,
    int columna,
    int fila
)
{
    double x0 = (double)columna;
    double y0 = (double)fila;

    double x1 = x0 + 1.0;
    double y1 = y0 + 1.0;

    /* Cuatro esquinas iniciales */
    Color a = trazar_muestra(
        escena, x0, y0
    );

    Color b = trazar_muestra(
        escena, x1, y0
    );

    Color c = trazar_muestra(
        escena, x0, y1
    );

    Color d = trazar_muestra(
        escena, x1, y1
    );

    return muestreo_adaptativo(
        escena,
        x0, y0,
        x1, y1,
        a, b, c, d,
        0
    );
}