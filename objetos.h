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

typedef struct {
    Vec3 posicion;
    double intensidad;
} Luz;

/* ------------------------------------------------------------------ */
/* Materiales                                                          */
/* ------------------------------------------------------------------ */

/*
 * Propiedades opticas de la superficie. Cada objeto tiene su propio
 * material, de modo que reacciona de forma distinta ante las fuentes
 * de luz:
 *
 *   ka  coeficiente de reflexion ambiental  -> componente ambiente
 *   kd  coeficiente de reflexion difusa     -> ley de Lambert
 *   ks  coeficiente de reflexion especular  -> brillo de Phong
 *   kn  exponente de brillo                 -> tamano del brillo
 *
 * Modelo aplicado por raytracing.c:
 *
 *   I = ka*Ia + SUM_luz( kd*Ip*Fatt*(N.L) ) + SUM_luz( ks*Ip*Fatt*(R.V)^kn )
 */
typedef struct {
    double ka;
    double kd;
    double ks;
    double kn;
} Material;

/* Valores por defecto usados cuando el archivo no especifica material. */
#define MATERIAL_KA_DEFECTO  0.3
#define MATERIAL_KD_DEFECTO  0.8
#define MATERIAL_KS_DEFECTO  0.6
#define MATERIAL_KN_DEFECTO  48.0

/* ------------------------------------------------------------------ */
/* Tipos de objeto                                                     */
/* ------------------------------------------------------------------ */

typedef enum {
    TIPO_ESFERA = 0,
    TIPO_POLIGONO = 1
} TipoObjeto;

typedef struct {
    Vec3    centro;
    double  radio;
    Color   color;
    Material material;
} Esfera;

typedef struct {
    Vec3 *vertices;
    int num_vertices;
    Vec3 normal;
    Color color;
    Material material;
} Poligono;

/*
 * Objeto "etiquetado": el campo `tipo` indica que miembro de la union es
 * valido. Nunca se accede a un miembro distinto del que indique `tipo`.
 */
typedef struct {
    TipoObjeto tipo;
    union {
        Esfera esfera;
        Poligono poligono;
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

    /*
     * Intensidad de la iluminacion ambiental de la escena (Ia).
     * Se lee con la palabra clave "ambiente"; su valor por defecto
     * conserva el resultado del modelo original (Ka * Ia = 0.15).
     */
    double ambiente;

    Objeto *objetos;      /* arreglo dinamico de objetos            */
    int     num_objetos;
    int     cap_objetos;

    /* Fuentes de luz de la escena */
    Luz *luces;
    int num_luces;
    int cap_luces;

    char error[256];      /* descripcion del ultimo error de lectura */
} Escena;

#endif /* OBJETOS_H */
