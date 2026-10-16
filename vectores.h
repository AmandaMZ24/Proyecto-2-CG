#ifndef VECTORES_H
#define VECTORES_H

#include "objetos.h"

/* Operaciones basicas con vectores */
Vec3 vec_restar(Vec3 a, Vec3 b);
Vec3 vec_sumar(Vec3 a, Vec3 b);
Vec3 vec_escalar(Vec3 v, double t);
double vec_punto(Vec3 a, Vec3 b);

/* Operaciones para iluminacion */
double vec_longitud(Vec3 v);
Vec3 vec_normalizar(Vec3 v);

#endif