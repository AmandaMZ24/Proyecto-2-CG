#include <stddef.h>
#include <math.h>

#include "discos.h"
#include "vectores.h"
#include "poligonos.h"

int intersecar_disco(
    Vec3 origen,
    Vec3 direccion,
    const Disco *disco,
    double *t
)
{
    const double EPSILON = 1e-9;

    /* Validar los parametros */
    if (disco == NULL ||
        t == NULL ||
        disco->radio <= 0.0) {
        return 0;
    }

    /* Validar la normal del disco */
    if (vec_longitud(disco->normal) <= EPSILON) {
        return 0;
    }

    Vec3 normal = vec_normalizar(disco->normal);

    double t_plano;

    /* Calcular la interseccion con el plano */
    if (!intersecar_plano(
            origen,
            direccion,
            disco->centro,
            normal,
            &t_plano)) {
        return 0;
    }

    /* Calcular el punto de impacto */
    Vec3 punto = vec_sumar(
        origen,
        vec_escalar(direccion, t_plano)
    );

    /* Vector desde el centro hasta el impacto */
    Vec3 diferencia = vec_restar(
        punto,
        disco->centro
    );

    /* Calcular la distancia al cuadrado */
    double distancia2 = vec_punto(
        diferencia,
        diferencia
    );

    double radio2 = disco->radio * disco->radio;

    /* Verificar si el impacto pertenece al disco */
    if (distancia2 > radio2 + EPSILON) {
        return 0;
    }

    /* Guardar la interseccion valida */
    *t = t_plano;

    return 1;
}