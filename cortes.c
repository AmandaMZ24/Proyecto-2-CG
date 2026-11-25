#include <stddef.h>
#include "cortes.h"

int punto_valido_cortes(
    Vec3 punto,
    const Objeto *objeto
)
{
    const double EPSILON = 1e-9;

    if (objeto == NULL ||
    objeto->num_planos_corte < 0 ||
    objeto->num_planos_corte > MAX_PLANOS_CORTE) {

    return 0;
}

    for (int i = 0; i < objeto->num_planos_corte; i++) {

        const PlanoCorte *plano =
            &objeto->planos_corte[i];

        double resultado =
              plano->A * punto.x
            + plano->B * punto.y
            + plano->C * punto.z
            + plano->D;

        if (resultado > EPSILON) {
            return 0;
        }
    }

    return 1;
}