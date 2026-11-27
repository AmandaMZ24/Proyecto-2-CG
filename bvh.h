#ifndef BVH_H
#define BVH_H

#include "objetos.h"

/* Caja delimitadora alineada con los ejes */
typedef struct {
    Vec3 minimo;
    Vec3 maximo;
} AABB;

/* Nodo de la jerarquia */
typedef struct {
    AABB caja;

    int izquierdo;
    int derecho;

    int inicio;
    int cantidad;
} NodoBVH;

/* Arbol completo */
typedef struct BVH {
    NodoBVH *nodos;
    int *indices;

    int num_nodos;
    int num_indices;
    int raiz;

    /* Primitivas que no pertenecen al BVH */
    int *indices_restantes;
    int num_restantes;

} BVH;

/* Construccion y liberacion */
BVH *bvh_construir(const Escena *escena);

void bvh_liberar(BVH *bvh);

/* Interseccion rayo-caja */
int aabb_intersecar(
    const AABB *caja,
    Vec3 origen,
    Vec3 direccion,
    double t_min,
    double t_max
);

#endif