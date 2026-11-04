#ifndef POLIGONOS_H
#define POLIGONOS_H

#include "objetos.h"

/*
 * Calcula la interseccion entre un rayo y un plano.
 *
 * origen: origen del rayo.
 * direccion: direccion del rayo.
 * punto_plano: punto perteneciente al plano.
 * normal: vector normal del plano.
 * t: parametro de interseccion calculado.
 *
 * Retorna:
 * 1 si existe una interseccion valida hacia adelante.
 * 0 si el rayo es paralelo al plano o no hay
 *   una interseccion valida hacia adelante.
 */
int intersecar_plano(
    Vec3 origen,
    Vec3 direccion,
    Vec3 punto_plano,
    Vec3 normal,
    double *t
);

int punto_en_poligono(
    Vec3 punto, 
    const Poligono *poligono
);

/*
 * Calcula la normal unitaria de un poligono.
 *
 * Retorna 1 si encuentra una normal valida.
 * Retorna 0 si los vertices son degenerados.
 */
int calcular_normal_poligono(
    const Poligono *poligono,
    Vec3 *normal
);

/*
 * Determina si un rayo intersecta un poligono.
 *
 * Retorna 1 si existe una interseccion valida.
 * Retorna 0 en caso contrario.
 *
 * Guarda en t la distancia parametrica.
 */
int intersecar_poligono(
    Vec3 origen,
    Vec3 direccion,
    const Poligono *poligono,
    double *t
);

/*
 * Verifica que todos los vertices de un poligono
 * pertenezcan al mismo plano.
 *
 * Retorna:
 * 1 si los vertices son coplanares.
 * 0 si no lo son o el poligono es invalido.
 */
int validar_coplanaridad(
    const Poligono *poligono
);

#endif