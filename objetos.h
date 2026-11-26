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
#define MAX_PLANOS_CORTE 8


/* ------------------------------------------------------------------ */
/* Vectores y colores                                                  */
/* ------------------------------------------------------------------ */

typedef struct {
    double x;
    double y;
    double z;
} Vec3;

typedef struct {
    double A;
    double B;
    double C;
    double D;
} PlanoCorte;

typedef struct {
    double r;
    double g;
    double b;
} Color;

typedef struct {
    int ancho;
    int alto;
    unsigned char *pixeles;
} Textura;

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
 *   reflejo        0 = superficie opaca, 1 = espejo perfecto.
 *                  Fraccion del color que se sustituye por el rayo
 *                  reflejado (recursion).
 *   transparencia  0 = superficie opaca, 1 = deja pasar toda la luz.
 *                  Fraccion del color que se sustituye por lo que se
 *                  ve detras de la superficie, SIN refraccion (el rayo
 *                  no se desvia); el color del objeto tiñe ese fondo
 *                  (cristal de color).
 *
 * Modelo aplicado por raytracing.c:
 *
 *   local = ka*Ia + SUM_luz( Fatt*( kd*Ip*(N.L) + ks*Ip*(R.V)^kn ) )
 *
 *   a_traves  = color visto detras del objeto (mismo rayo, sin
 *               refraccion), multiplicado por el color base del objeto
 *   reflejado = color del rayo reflejado sobre la normal
 *
 *   color = (1 - reflejo) * [ (1 - transparencia) * local
 *                           +   transparencia     * a_traves ]
 *         + reflejo * reflejado
 */
typedef struct {
    double ka;
    double kd;
    double ks;
    double kn;
    double reflejo;
    double transparencia;
} Material;

/* Valores por defecto usados cuando el archivo no especifica material. */
#define MATERIAL_KA_DEFECTO         0.3
#define MATERIAL_KD_DEFECTO         0.8
#define MATERIAL_KS_DEFECTO         0.6
#define MATERIAL_KN_DEFECTO         48.0
#define MATERIAL_REFLEJO_DEFECTO    0.0
#define MATERIAL_TRANSPARENCIA_DEFECTO 0.0

/* ------------------------------------------------------------------ */
/* Tipos de objeto                                                     */
/* ------------------------------------------------------------------ */

typedef enum {
    TIPO_ESFERA = 0,
    TIPO_POLIGONO = 1,
    TIPO_DISCO = 2,
    TIPO_ELIPSE = 3,
    TIPO_CILINDRO = 4,
    TIPO_CONO = 5,
    TIPO_CUADRATICA = 6
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

typedef struct {
    Vec3 centro;
    Vec3 normal;
    double radio;
    Color color;
    Material material;
} Disco;

typedef struct {
    Vec3 foco1;
    Vec3 foco2;
    Vec3 normal;
    double distancia_maxima;
    Color color;
    Material material;
} Elipse;

typedef struct {
    Vec3 ancla;
    Vec3 eje;
    double radio;
    double d_min;
    double d_max;
    Color color;
    Material material;
} Cilindro;

typedef struct {
    Vec3 vertice;
    Vec3 eje;
    double m;
    double n;
    double d_min;
    double d_max;
    Color color;
    Material material;
} Cono;

typedef struct {
    double A;
    double B;
    double C;
    double D;
    double E;
    double F;
    double G;
    double H;
    double I;
    double J;

    Color color;
    Material material;

} Cuadratica;

/*
 * Objeto "etiquetado": el campo `tipo` indica que miembro de la union es
 * valido. Nunca se accede a un miembro distinto del que indique `tipo`.
 */
typedef struct {
    TipoObjeto tipo;
    union {
        Esfera esfera;
        Poligono poligono;
        Disco disco;
        Elipse elipse;
        Cilindro cilindro;
        Cono cono;
        Cuadratica cuadratica;
    } u;
    PlanoCorte planos_corte[MAX_PLANOS_CORTE];
    int num_planos_corte;
    int textura;
    Vec3 textura_g;
    Vec3 textura_norte;

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

    Textura *texturas;
    int num_texturas;
    int cap_texturas;
    
    int antialiasing; /* 0 = desactivado, 1 = activado */

    char error[256];      /* descripcion del ultimo error de lectura */
} Escena;

#endif /* OBJETOS_H */
