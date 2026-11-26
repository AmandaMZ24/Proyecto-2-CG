#include <stddef.h>
#include <math.h>

#include "elipses.h"
#include "vectores.h"
#include "poligonos.h"

int intersecar_elipse(
    Vec3 origen,
    Vec3 direccion,
    const Elipse *elipse,
    double *t
)
{
    const double EPSILON = 1e-9;

    /* Validar los parametros */
    if (elipse == NULL ||
        t == NULL ||
        elipse->distancia_maxima <= 0.0) {
        return 0;
    }

    /* Validar la normal */
    if (vec_longitud(elipse->normal) <= EPSILON) {
        return 0;
    }

    Vec3 normal = vec_normalizar(elipse->normal);

    /* Calcular el vector entre los focos */
    Vec3 diferencia_focos = vec_restar(
        elipse->foco2,
        elipse->foco1
    );

    double distancia_focos =
        vec_longitud(diferencia_focos);

    /* Evitar elipses degeneradas */
    if (elipse->distancia_maxima <=
        distancia_focos + EPSILON) {
        return 0;
    }

    /* Verificar que ambos focos sean coplanares */
    if (fabs(vec_punto(normal, diferencia_focos))
        > EPSILON) {
        return 0;
    }

    double t_plano;

    /* Interseccion entre el rayo y el plano */
    if (!intersecar_plano(
            origen,
            direccion,
            elipse->foco1,
            normal,
            &t_plano)) {
        return 0;
    }

    /* Calcular el punto de impacto */
    Vec3 punto = vec_sumar(
        origen,
        vec_escalar(direccion, t_plano)
    );

    /* Calcular las distancias a los focos */
    double d1 = vec_longitud(
        vec_restar(punto, elipse->foco1)
    );

    double d2 = vec_longitud(
        vec_restar(punto, elipse->foco2)
    );

    /* Verificar si el punto pertenece a la elipse */
    if (d1 + d2 >
        elipse->distancia_maxima + EPSILON) {
        return 0;
    }

    /* Guardar la interseccion valida */
    *t = t_plano;

    return 1;
}