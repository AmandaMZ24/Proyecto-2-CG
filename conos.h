#ifndef CONOS_H
#define CONOS_H

#include "objetos.h"

int intersecar_cono(
    Vec3 origen,
    Vec3 direccion,
    const Cono *cono,
    double *t
);

Vec3 normal_cono(
    Vec3 punto,
    const Cono *cono
);

#endif