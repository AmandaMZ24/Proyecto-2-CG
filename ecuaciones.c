#include <math.h>
#include <stddef.h>
#include "ecuaciones.h"

int resolver_cuadratica(
    double A,
    double B,
    double C,
    double soluciones[2]
)
{
    const double EPSILON = 1e-12;

    if (soluciones == NULL ||
        !isfinite(A) ||
        !isfinite(B) ||
        !isfinite(C)) {
        return 0;
    }

    /* Normalizar los coeficientes */
    double escala = fmax(
        fabs(A),
        fmax(fabs(B), fabs(C))
    );

    if (escala == 0.0) {
        return 0;
    }

    A /= escala;
    B /= escala;
    C /= escala;

    /* Caso lineal */
    if (fabs(A) <= EPSILON) {

        if (fabs(B) <= EPSILON) {
            return 0;
        }

        soluciones[0] = -C / B;

        return 1;
    }

    double discriminante = B * B - 4.0 * A * C;

    /* No existen soluciones reales */
    if (discriminante < 0.0) {
        return 0;
    }

    /* Una unica solucion */
    if (discriminante == 0.0) {

        soluciones[0] = -B / (2.0 * A);

        return 1;
    }

    /* Dos soluciones reales */
    double raiz = sqrt(discriminante);

    /* Variante numericamente mas estable */
    double q = -0.5 * (B + copysign(raiz, B));

    soluciones[0] = q / A;
    soluciones[1] = C / q;

    /* Ordenar de menor a mayor */
    if (soluciones[0] > soluciones[1]) {

        double temporal = soluciones[0];

        soluciones[0] = soluciones[1];
        soluciones[1] = temporal;
    }

    return 2;
}