#include <stddef.h>
#include <math.h>

#include "conos.h"
#include "vectores.h"

#define EPSILON 1e-9

int intersecar_cono(
    Vec3 origen,
    Vec3 direccion,
    const Cono *cono,
    double *t
)
{
    if (cono == NULL ||
        t == NULL ||
        cono->m <= EPSILON ||
        cono->n <= EPSILON ||
        cono->d_min >= cono->d_max ||
        vec_longitud(cono->eje) <= EPSILON) {

        return 0;
    }

    Vec3 Q = vec_normalizar(cono->eje);

    Vec3 W = vec_restar(
        origen,
        cono->vertice
    );

    /* Razon de apertura del cono */
    double k = cono->n / cono->m;

    double factor = 1.0 + k * k;

    double DQ = vec_punto(direccion, Q);
    double WQ = vec_punto(W, Q);

    /* Coeficientes de la ecuacion cuadratica */
    double A = vec_punto(direccion, direccion)
             - factor * DQ * DQ;

    double B = 2.0 * (
        vec_punto(W, direccion)
        - factor * WQ * DQ
    );

    double C = vec_punto(W, W)
             - factor * WQ * WQ;

    double soluciones[2];
    int cantidad;

    /* Caso especial: ecuacion lineal */
    if (fabs(A) <= EPSILON) {

        if (fabs(B) <= EPSILON) {
            return 0;
        }

        soluciones[0] = -C / B;
        cantidad = 1;

    } else {

        double discriminante = B * B - 4.0 * A * C;

        if (discriminante < -EPSILON) {
            return 0;
        }

        discriminante = fmax(0.0, discriminante);

        double raiz = sqrt(discriminante);

        soluciones[0] = (-B - raiz) / (2.0 * A);
        soluciones[1] = (-B + raiz) / (2.0 * A);

        /* Ordenar las soluciones */
        if (soluciones[0] > soluciones[1]) {

            double temporal = soluciones[0];

            soluciones[0] = soluciones[1];
            soluciones[1] = temporal;
        }

        cantidad = 2;
    }

    /* Evaluar las intersecciones */
    for (int i = 0; i < cantidad; i++) {

        double candidato = soluciones[i];

        if (candidato <= EPSILON) {
            continue;
        }

        Vec3 punto = vec_sumar(
            origen,
            vec_escalar(direccion, candidato)
        );

        double d = vec_punto(
            vec_restar(punto, cono->vertice),
            Q
        );

        /* Aplicar limites longitudinales */
        if (d < cono->d_min - EPSILON ||
            d > cono->d_max + EPSILON) {

            continue;
        }

        /* En el vertice no existe normal unica */
        if (fabs(d) <= EPSILON) {
            continue;
        }

        *t = candidato;
        return 1;
    }

    return 0;
}

Vec3 normal_cono(
    Vec3 punto,
    const Cono *cono
)
{
    Vec3 Q = vec_normalizar(cono->eje);

    Vec3 W = vec_restar(
        punto,
        cono->vertice
    );

    double k = cono->n / cono->m;

    double d = vec_punto(W, Q);

    /* Gradiente de la superficie implicita */
    Vec3 normal = vec_restar(
        W,
        vec_escalar(
            Q,
            (1.0 + k * k) * d
        )
    );

    return vec_normalizar(normal);
}