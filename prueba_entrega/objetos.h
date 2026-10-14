/*
 * objetos.h
 *
 * Definicion de los tipos basicos (vectores, colores) y de la jerarquia de
 * objetos de la escena. El diseno esta preparado para agregar nuevos tipos de
 * objetos en el futuro (planos, cajas, ...) sin tocar el codigo que recorre
 * la escena: basta con agregar un valor al enum TipoObjeto, un miembro a la
 * union y una rama en cada switch que use dicha union.
 */
#ifndef OBJETOS_H
#define OBJETOS_H

/* ------------------------------------------------------------------ */
/* Vectores y colores                                                  */
/* ------------------------------------------------------------------ */

typedef struct {
    double x;
    double y;
    double z;
} Vec3;

typedef struct {
    double r;
    double g;
    double b;
} Color;

/* ------------------------------------------------------------------ */
/* Tipos de objeto                                                     */
/* ------------------------------------------------------------------ */

typedef enum {
    TIPO_ESFERA = 0
    /* Futuros tipos:
     * , TIPO_PLANO
     * , TIPO_CAJA
     */
} TipoObjeto;

typedef struct {
    Vec3   centro;
    double radio;
    Color  color;
} Esfera;

/*
 * Objeto "etiquetado": el campo `tipo` indica que miembro de la union es
 * valido. Nunca se accede a un miembro distinto del que indique `tipo`.
 */
typedef struct {
    TipoObjeto tipo;
    union {
        Esfera esfera;
        /* Futuros objetos:
         * Plano plano;
         * Caja  caja;
         */
    } u;
} Objeto;

/* ------------------------------------------------------------------ */
/* Escena                                                              */
/* ------------------------------------------------------------------ */

/*
 * La ventana es un rectangulo en el plano de proyeccion (z = 0). El ojo es el
 * punto de observacion; los rayos se disparan desde el ojo hacia cada pixel
 * de la ventana.
 */
typedef struct {
    Vec3   ojo;
    double ventana_xmin;
    double ventana_ymin;
    double ventana_xmax;
    double ventana_ymax;

    int ancho;   /* resolucion del framebuffer, en pixeles */
    int alto;

    Color fondo; /* color cuando el rayo no golpea ningun objeto */

    Objeto *objetos;      /* arreglo dinamico de objetos            */
    int     num_objetos;
    int     cap_objetos;

    char error[256];      /* descripcion del ultimo error de lectura */
} Escena;

#endif /* OBJETOS_H */
