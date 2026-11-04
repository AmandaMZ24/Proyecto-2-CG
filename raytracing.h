#ifndef RAYTRACING_H
#define RAYTRACING_H
#include "objetos.h"

/*
 * Devuelve 1 si el rayo golpea algun objeto de la escena hacia adelante.
 * Guarda en *indice el objeto mas cercano y en *t su parametro.
 * Devuelve 0 si no hay interseccion (los parametros no se modifican).
 */
int First_Intersection(const Escena *escena, Vec3 origen, Vec3 direccion,
                       int *indice, double *t);

/*
 * Calcula el color visible de un rayo: interseccion mas cercana +
 * modelo de iluminacion completo (ambiental, difusa de Lambert,
 * especular de Phong, sombras) segun el material de cada objeto.
 * Si el rayo no golpea nada devuelve escena->fondo.
 */
Color De_Que_Color(const Escena *escena, Vec3 origen, Vec3 direccion);

/* Calcula el color visible de un pixel (fila 0 = parte superior). */
Color trazar_pixel(const Escena *escena, int columna, int fila);

/* Devuelve 1 si el rayo intersecta la esfera hacia adelante y escribe t. */
int intersecar_esfera(Vec3 origen, Vec3 direccion, const Esfera *esfera, double *t);
#endif
