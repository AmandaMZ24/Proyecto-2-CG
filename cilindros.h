#ifndef CILINDROS_H
#define CILINDROS_H

#include "objetos.h"

int intersecar_cilindro(
    Vec3 origen,
    Vec3 direccion,
    const Cilindro *cilindro,
    double *t
);

Vec3 normal_cilindro(
    Vec3 punto,
    const Cilindro *cilindro
);

#endif