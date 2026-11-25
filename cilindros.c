#include <stddef.h>
#include <math.h>

#include "cilindros.h"
#include "vectores.h"

#define EPSILON 1e-9

int intersecar_cilindro(
    Vec3 origen,
    Vec3 direccion,
    const Cilindro *cilindro,
    double *t
)
{
    if (cilindro == NULL ||
        t == NULL ||
        cilindro->radio <= 0.0 ||
        cilindro->d_min >= cilindro->d_max ||
        vec_longitud(cilindro->eje) <= EPSILON) {

        return 0;
    }

    /* Vector unitario del eje */
    Vec3 Q = vec_normalizar(cilindro->eje);

    /* Vector desde el ancla hasta el origen */
    Vec3 W = vec_restar(origen, cilindro->ancla);

    double DQ = vec_punto(direccion, Q);
    double WQ = vec_punto(W, Q);

    /* Componentes perpendiculares al eje */
    Vec3 D_perp = vec_restar(
        direccion,
        vec_escalar(Q, DQ)
    );

    Vec3 W_perp = vec_restar(
        W,
        vec_escalar(Q, WQ)
    );

    /* Coeficientes de la ecuacion cuadratica */
    double A = vec_punto(D_perp, D_perp);

    double B = 2.0 * vec_punto(D_perp, W_perp);

    double C = vec_punto(W_perp, W_perp)
             - cilindro->radio * cilindro->radio;

    /* Rayo paralelo al eje del cilindro */
    if (fabs(A) <= EPSILON) {
        return 0;
    }

    double discriminante = B * B - 4.0 * A * C;

    if (discriminante < -EPSILON) {
        return 0;
    }

    /* Evitar errores por redondeo */
    discriminante = fmax(0.0, discriminante);

    double raiz = sqrt(discriminante);

    double soluciones[2] = {
        (-B - raiz) / (2.0 * A),
        (-B + raiz) / (2.0 * A)
    };

    /* Evaluar primero la interseccion mas cercana */
    for (int i = 0; i < 2; i++) {

        double candidato = soluciones[i];

        if (candidato <= EPSILON) {
            continue;
        }

        Vec3 punto = vec_sumar(
            origen,
            vec_escalar(direccion, candidato)
        );

        /* Distancia longitudinal */
        double d = vec_punto(
            vec_restar(punto, cilindro->ancla),
            Q
        );

        /* Aplicar los limites del cilindro */
        if (d < cilindro->d_min - EPSILON ||
            d > cilindro->d_max + EPSILON) {

            continue;
        }

        *t = candidato;
        return 1;
    }

    return 0;
}

Vec3 normal_cilindro(
    Vec3 punto,
    const Cilindro *cilindro
)
{
    Vec3 Q = vec_normalizar(cilindro->eje);

    Vec3 W = vec_restar(
        punto,
        cilindro->ancla
    );

    /* Proyeccion sobre el eje */
    double d = vec_punto(W, Q);

    Vec3 M = vec_sumar(
        cilindro->ancla,
        vec_escalar(Q, d)
    );

    /* Vector radial normalizado */
    return vec_normalizar(
        vec_restar(punto, M)
    );
}