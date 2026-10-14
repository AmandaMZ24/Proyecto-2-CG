#ifndef RAYTRACING_H
#define RAYTRACING_H
#include "objetos.h"
/* Calcula el color visible de un pixel (fila 0 = parte superior). */
Color trazar_pixel(const Escena *escena, int columna, int fila);
/* Devuelve 1 si el rayo intersecta la esfera hacia adelante y escribe t. */
int intersecar_esfera(Vec3 origen, Vec3 direccion, const Esfera *esfera, double *t);
#endif
