#include "vectores.h"
#include <math.h>

/* Suma de vectores */
Vec3 vec_sumar(Vec3 a, Vec3 b)
{
    return (Vec3){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

/* Resta de vectores */
Vec3 vec_restar(Vec3 a, Vec3 b)
{
    return (Vec3){
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

/* Multiplicacion por un escalar */
Vec3 vec_escalar(Vec3 v, double t)
{
    return (Vec3){
        v.x * t,
        v.y * t,
        v.z * t
    };
}

/* Producto punto */
double vec_punto(Vec3 a, Vec3 b)
{
    return a.x * b.x
         + a.y * b.y
         + a.z * b.z;
}

/* Longitud de un vector */
double vec_longitud(Vec3 v)
{
    return sqrt(vec_punto(v, v));
}

/* Normalizar un vector */
Vec3 vec_normalizar(Vec3 v)
{
    double longitud = vec_longitud(v);

    if (longitud <= 0.0)
    {
        return (Vec3){0.0, 0.0, 0.0};
    }

    return vec_escalar(v, 1.0 / longitud);
}

/* Producto cruz */
Vec3 vec_cruz(Vec3 a, Vec3 b)
{
    Vec3 resultado;
    resultado.x = a.y * b.z - a.z * b.y;
    resultado.y = a.z * b.x - a.x * b.z;
    resultado.z = a.x * b.y - a.y * b.x;
    return resultado;
}
