#ifndef CUADRATICAS_H
#define CUADRATICAS_H

#include "objetos.h"

int intersecciones_cuadratica(
    Vec3 origen,
    Vec3 direccion,
    const Cuadratica *superficie,
    double soluciones[2]
);

Vec3 normal_cuadratica(
    Vec3 punto,
    const Cuadratica *superficie
);

#endif