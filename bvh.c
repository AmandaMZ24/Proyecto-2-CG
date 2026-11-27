#include <stdlib.h>
#include <math.h>
#include <float.h>

#include "bvh.h"

int aabb_intersecar(
    const AABB *caja,
    Vec3 origen,
    Vec3 direccion,
    double t_min,
    double t_max
)
{
    double o[3] = {
        origen.x,
        origen.y,
        origen.z
    };

    double d[3] = {
        direccion.x,
        direccion.y,
        direccion.z
    };

    double minimo[3] = {
        caja->minimo.x,
        caja->minimo.y,
        caja->minimo.z
    };

    double maximo[3] = {
        caja->maximo.x,
        caja->maximo.y,
        caja->maximo.z
    };

    for (int eje = 0; eje < 3; eje++) {

        /* Rayo paralelo a las caras de la caja */
        if (d[eje] == 0.0) {

            if (o[eje] < minimo[eje] ||
                o[eje] > maximo[eje]) {

                return 0;
            }

            continue;
        }

        double t1 =
            (minimo[eje] - o[eje]) / d[eje];

        double t2 =
            (maximo[eje] - o[eje]) / d[eje];

        /* Ordenar entrada y salida */
        if (t1 > t2) {

            double temporal = t1;

            t1 = t2;
            t2 = temporal;
        }

        t_min = fmax(t_min, t1);
        t_max = fmin(t_max, t2);

        /* No existe intervalo de interseccion */
        if (t_max < t_min) {

            return 0;
        }
    }

    return 1;
}

static AABB caja_poligono(const Poligono *p)
{
    AABB caja;

    caja.minimo = (Vec3){DBL_MAX, DBL_MAX, DBL_MAX};
    caja.maximo = (Vec3){-DBL_MAX, -DBL_MAX, -DBL_MAX};

    for (int i = 0; i < p->num_vertices; i++) {

        Vec3 v = p->vertices[i];

        caja.minimo.x = fmin(caja.minimo.x, v.x);
        caja.minimo.y = fmin(caja.minimo.y, v.y);
        caja.minimo.z = fmin(caja.minimo.z, v.z);

        caja.maximo.x = fmax(caja.maximo.x, v.x);
        caja.maximo.y = fmax(caja.maximo.y, v.y);
        caja.maximo.z = fmax(caja.maximo.z, v.z);
    }

    /* Margen para evitar problemas de precision */
    const double epsilon = 1e-7;

    caja.minimo.x -= epsilon;
    caja.minimo.y -= epsilon;
    caja.minimo.z -= epsilon;

    caja.maximo.x += epsilon;
    caja.maximo.y += epsilon;
    caja.maximo.z += epsilon;

    return caja;
}

static AABB unir_cajas(AABB a, AABB b)
{
    AABB resultado;

    resultado.minimo = (Vec3){
        fmin(a.minimo.x, b.minimo.x),
        fmin(a.minimo.y, b.minimo.y),
        fmin(a.minimo.z, b.minimo.z)
    };

    resultado.maximo = (Vec3){
        fmax(a.maximo.x, b.maximo.x),
        fmax(a.maximo.y, b.maximo.y),
        fmax(a.maximo.z, b.maximo.z)
    };

    return resultado;
}

static Vec3 centro_caja(AABB caja)
{
    return (Vec3){
        (caja.minimo.x + caja.maximo.x) * 0.5,
        (caja.minimo.y + caja.maximo.y) * 0.5,
        (caja.minimo.z + caja.maximo.z) * 0.5
    };
}

static int eje_mayor(AABB caja)
{
    double dx = caja.maximo.x - caja.minimo.x;
    double dy = caja.maximo.y - caja.minimo.y;
    double dz = caja.maximo.z - caja.minimo.z;

    if (dx >= dy && dx >= dz)
        return 0;

    if (dy >= dz)
        return 1;

    return 2;
}

typedef struct {
    int indice;
    AABB caja;
    Vec3 centro;
} ReferenciaBVH;

static int comparar_x(const void *a, const void *b)
{
    const ReferenciaBVH *ra = a;
    const ReferenciaBVH *rb = b;

    return (ra->centro.x > rb->centro.x) -
           (ra->centro.x < rb->centro.x);
}

static int comparar_y(const void *a, const void *b)
{
    const ReferenciaBVH *ra = a;
    const ReferenciaBVH *rb = b;

    return (ra->centro.y > rb->centro.y) -
           (ra->centro.y < rb->centro.y);
}

static int comparar_z(const void *a, const void *b)
{
    const ReferenciaBVH *ra = a;
    const ReferenciaBVH *rb = b;

    return (ra->centro.z > rb->centro.z) -
           (ra->centro.z < rb->centro.z);
}

#define BVH_MAX_OBJETOS_HOJA 4

static int construir_nodo(
    BVH *bvh,
    ReferenciaBVH *refs,
    int inicio,
    int cantidad
)
{
    int indice_nodo = bvh->num_nodos++;

    NodoBVH *nodo = &bvh->nodos[indice_nodo];

    nodo->izquierdo = -1;
    nodo->derecho = -1;
    nodo->inicio = inicio;
    nodo->cantidad = cantidad;

    /* Caja que contiene todos los objetos del nodo */
    AABB caja = refs[inicio].caja;

    for (int i = inicio + 1;
         i < inicio + cantidad;
         i++) {

        caja = unir_cajas(caja, refs[i].caja);
    }

    nodo->caja = caja;

    /* Caso base: nodo hoja */
    if (cantidad <= BVH_MAX_OBJETOS_HOJA) {

        for (int i = inicio;
             i < inicio + cantidad;
             i++) {

            bvh->indices[i] = refs[i].indice;
        }

        return indice_nodo;
    }

    /* Elegir el eje de mayor extension */
    int eje = eje_mayor(caja);

    /* Ordenar los objetos segun su posicion */
    if (eje == 0) {

        qsort(
            refs + inicio,
            cantidad,
            sizeof(ReferenciaBVH),
            comparar_x
        );

    } else if (eje == 1) {

        qsort(
            refs + inicio,
            cantidad,
            sizeof(ReferenciaBVH),
            comparar_y
        );

    } else {

        qsort(
            refs + inicio,
            cantidad,
            sizeof(ReferenciaBVH),
            comparar_z
        );
    }

    /* Dividir los objetos en dos grupos */
    int mitad = cantidad / 2;

    nodo->cantidad = 0;

    nodo->izquierdo = construir_nodo(
        bvh,
        refs,
        inicio,
        mitad
    );

    nodo->derecho = construir_nodo(
        bvh,
        refs,
        inicio + mitad,
        cantidad - mitad
    );

    return indice_nodo;
}

BVH *bvh_construir(const Escena *escena)
{
    if (escena == NULL ||
        escena->objetos == NULL ||
        escena->num_objetos <= 0) {

        return NULL;
    }

    int cantidad = 0;

    /* Contar los poligonos */
    for (int i = 0; i < escena->num_objetos; i++) {

        if (escena->objetos[i].tipo == TIPO_POLIGONO) {
            cantidad++;
        }
    }

    if (cantidad == 0)
        return NULL;

    BVH *bvh = calloc(1, sizeof(BVH));

    if (bvh == NULL)
        return NULL;

    bvh->raiz = -1;
    bvh->num_indices = cantidad;

    bvh->num_restantes =
        escena->num_objetos - cantidad;

    bvh->nodos = malloc(
        (size_t)2 * cantidad * sizeof(NodoBVH)
    );

    bvh->indices = malloc(
        (size_t)cantidad * sizeof(int)
    );

    ReferenciaBVH *refs = malloc(
        (size_t)cantidad * sizeof(ReferenciaBVH)
    );

    /* Reservar espacio para las otras primitivas */
    if (bvh->num_restantes > 0) {

        bvh->indices_restantes = malloc(
            (size_t)bvh->num_restantes * sizeof(int)
        );
    }

    if (bvh->nodos == NULL ||
        bvh->indices == NULL ||
        refs == NULL ||
        (bvh->num_restantes > 0 &&
         bvh->indices_restantes == NULL)) {

        free(refs);
        bvh_liberar(bvh);

        return NULL;
    }

    int posicion = 0;
    int restante = 0;

    /* Clasificar los objetos de la escena */
    for (int i = 0; i < escena->num_objetos; i++) {

        const Objeto *objeto = &escena->objetos[i];

        if (objeto->tipo != TIPO_POLIGONO) {

            bvh->indices_restantes[restante++] = i;

            continue;
        }

        refs[posicion].indice = i;

        refs[posicion].caja =
            caja_poligono(&objeto->u.poligono);

        refs[posicion].centro =
            centro_caja(refs[posicion].caja);

        posicion++;
    }

    /* Construir el arbol */
    bvh->raiz = construir_nodo(
        bvh,
        refs,
        0,
        cantidad
    );

    free(refs);

    return bvh;
}

void bvh_liberar(BVH *bvh)
{
    if (bvh == NULL)
        return;

    free(bvh->nodos);
    free(bvh->indices);
    free(bvh->indices_restantes);

    free(bvh);
}