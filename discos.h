#ifndef DISCOS_H
#define DISCOS_H

#include "objetos.h"

/*
 * Determina si un rayo intersecta un disco.
 * Retorna 1 si existe una interseccion valida.
 * Retorna 0 en caso contrario.
 */
int intersecar_disco(
    Vec3 origen,
    Vec3 direccion,
    const Disco *disco,
    double *t
);

#endif