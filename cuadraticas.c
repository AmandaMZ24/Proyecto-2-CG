#include <stddef.h>

#include "cuadraticas.h"
#include "ecuaciones.h"
#include "vectores.h"

int intersecciones_cuadratica(
    Vec3 origen,
    Vec3 direccion,
    const Cuadratica *s,
    double soluciones[2]
)
{
    if (s == NULL || soluciones == NULL) {
        return 0;
    }

    double ex = origen.x;
    double ey = origen.y;
    double ez = origen.z;

    double dx = direccion.x;
    double dy = direccion.y;
    double dz = direccion.z;

    /* Coeficiente de t^2 */
    double a =
          s->A * dx * dx
        + s->B * dy * dy
        + s->C * dz * dz
        + s->D * dx * dy
        + s->E * dx * dz
        + s->F * dy * dz;

    /* Coeficiente de t */
    double b =
          2.0 * s->A * ex * dx
        + 2.0 * s->B * ey * dy
        + 2.0 * s->C * ez * dz

        + s->D * (ex * dy + ey * dx)
        + s->E * (ex * dz + ez * dx)
        + s->F * (ey * dz + ez * dy)

        + s->G * dx
        + s->H * dy
        + s->I * dz;

    /* Termino independiente */
    double c =
          s->A * ex * ex
        + s->B * ey * ey
        + s->C * ez * ez

        + s->D * ex * ey
        + s->E * ex * ez
        + s->F * ey * ez

        + s->G * ex
        + s->H * ey
        + s->I * ez
        + s->J;

    return resolver_cuadratica(
        a, b, c, soluciones
    );
}

Vec3 normal_cuadratica(
    Vec3 punto,
    const Cuadratica *s
)
{
    if (s == NULL) {
        return (Vec3){0.0, 0.0, 0.0};
    }

    double x = punto.x;
    double y = punto.y;
    double z = punto.z;

    /* Gradiente de la superficie */
    Vec3 normal = {

        2.0 * s->A * x
        + s->D * y
        + s->E * z
        + s->G,

        2.0 * s->B * y
        + s->D * x
        + s->F * z
        + s->H,

        2.0 * s->C * z
        + s->E * x
        + s->F * y
        + s->I
    };

    if (vec_longitud(normal) <= 1e-12) {
        return (Vec3){0.0, 0.0, 0.0};
    }

    return vec_normalizar(normal);
}