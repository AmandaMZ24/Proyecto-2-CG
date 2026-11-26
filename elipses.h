#ifndef ELIPSES_H
#define ELIPSES_H

#include "objetos.h"

/*
 * Determina si un rayo intersecta una elipse.
 * Retorna 1 si existe una interseccion valida.
 * Retorna 0 en caso contrario.
 */
int intersecar_elipse(
    Vec3 origen,
    Vec3 direccion,
    const Elipse *elipse,
    double *t
);

#endif