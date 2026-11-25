/*
 * escena.c
 *
 * Lectura y validacion del archivo de descripcion de la escena.
 * No imprime nada por stdout/stderr: los errores se guardan en la escena y
 * los decide imprimir (o no) quien llame a escena_leer().
 */
#include "escena.h"
#include "poligonos.h"
#include "vectores.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINEA     1024
#define MAX_TOKENS    64

/* ------------------------------------------------------------------ */
/* Inicializacion y utilidades de error                                */
/* ------------------------------------------------------------------ */

void escena_inicializar(Escena *escena)
{
    if (escena == NULL) {
        return;
    }

    memset(escena, 0, sizeof(*escena));

    /* Valores por defecto razonables; el archivo los pisa todos. */
    escena->ojo.x = 0.0;
    escena->ojo.y = 0.0;
    escena->ojo.z = -5.0;

    escena->ventana_xmin = -1.6;
    escena->ventana_ymin = -0.9;
    escena->ventana_xmax =  1.6;
    escena->ventana_ymax =  0.9;

    escena->ancho = 1008;
    escena->alto  = 567;

    escena->fondo.r = 0.0;
    escena->fondo.g = 0.0;
    escena->fondo.b = 0.0;

    /*
     * Intensidad ambiental por defecto. Conserva el resultado del
     * modelo original: Ka(0.3) * Ia(0.5) = 0.15.
     */
    escena->ambiente = 0.5;

    escena->cap_objetos = ESCENA_CAP_INICIAL;
    escena->objetos = (Objeto *)calloc((size_t)escena->cap_objetos,
    sizeof(Objeto));
    if (escena->objetos == NULL) {
        escena->cap_objetos = 0;
    }

    escena->cap_luces = 4;
    escena->num_luces = 0;
    escena->luces = (Luz *)calloc((size_t)escena->cap_luces, sizeof(Luz));
    if (escena->luces == NULL) {
        escena->cap_luces = 0;
    }
}

const char *escena_error(const Escena *escena)
{
    if (escena == NULL) {
        return "escena nula";
    }
    return escena->error;
}

static void fijar_error(Escena *e, int linea, const char *nombre,
                        const char *motivo)
{
    if (e == NULL || e->error[0] != '\0') {
        return; /* conserva el primer error encontrado */
    }

    if (linea > 0) {
        snprintf(e->error, sizeof(e->error), "%s (linea %d): %s",
                 nombre, linea, motivo);
    } else {
        snprintf(e->error, sizeof(e->error), "%s: %s", nombre, motivo);
    }
}

/* ------------------------------------------------------------------ */
/* Parsing de tokens                                                   */
/* ------------------------------------------------------------------ */

/* Divide la linea en tokens separados por espacios/tabuladores.
 * Devuelve el numero de tokens, o -1 si hay mas de MAX_TOKENS-1. */
static int tokenizar(char *linea, char *tokens[])
{
    int n = 0;
    char *tok = strtok(linea, " \t\r\n");

    while (tok != NULL) {
        if (n >= MAX_TOKENS - 1) {
            return -1;
        }
        tokens[n++] = tok;
        tok = strtok(NULL, " \t\r\n");
    }
    tokens[n] = NULL;
    return n;
}

/* Convierte un token a double. Devuelve 1 si es un numero valido. */
static int a_doble(const char *tok, double *salida)
{
    char *fin = NULL;
    double valor;

    if (tok == NULL || *tok == '\0') {
        return 0;
    }

    errno = 0;
    valor = strtod(tok, &fin);
    if (fin == tok || *fin != '\0' || errno == ERANGE || !isfinite(valor)) {
        return 0;
    }

    *salida = valor;
    return 1;
}

/* Convierte un token a entero. Devuelve 1 si es un entero valido. */
static int a_entero(const char *tok, int *salida)
{
    char *fin = NULL;
    long valor;

    if (tok == NULL || *tok == '\0') {
        return 0;
    }

    errno = 0;
    valor = strtol(tok, &fin, 10);
    if (fin == tok || *fin != '\0' || errno == ERANGE ||
        valor > 2147483647L || valor < -2147483648L) {
        return 0;
    }

    *salida = (int)valor;
    return 1;
}

/* Exige exactamente `esperados` tokens en la linea. */
static int arity_ok(Escena *e, int n, int esperados, int linea,
                    const char *nombre, const char *clave)
{
    if (n != esperados) {
        char motivo[160];
        snprintf(motivo, sizeof(motivo),
                 "\"%s\" espera %d argumento(s), se encontraron %d",
                 clave, esperados - 1, n - 1);
        fijar_error(e, linea, nombre, motivo);
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Materiales                                                          */
/* ------------------------------------------------------------------ */

void material_defecto(Material *material)
{
    if (material == NULL) {
        return;
    }

    material->ka = MATERIAL_KA_DEFECTO;
    material->kd = MATERIAL_KD_DEFECTO;
    material->ks = MATERIAL_KS_DEFECTO;
    material->kn = MATERIAL_KN_DEFECTO;
    material->reflejo = MATERIAL_REFLEJO_DEFECTO;
    material->transparencia = MATERIAL_TRANSPARENCIA_DEFECTO;
}

/*
 * Valida los coeficientes de un material.
 * Devuelve 1 si todos los valores son utilizables.
 */
static int material_valido(const Material *m)
{
    if (m == NULL) {
        return 0;
    }

    if (!isfinite(m->ka) || m->ka < 0.0 || m->ka > 1.0 ||
        !isfinite(m->kd) || m->kd < 0.0 || m->kd > 1.0 ||
        !isfinite(m->ks) || m->ks < 0.0 || m->ks > 1.0 ||
        !isfinite(m->kn) || m->kn <= 0.0 ||
        !isfinite(m->reflejo) || m->reflejo < 0.0 || m->reflejo > 1.0 ||
        !isfinite(m->transparencia) || m->transparencia < 0.0 ||
        m->transparencia > 1.0) {
        return 0;
    }

    return 1;
}

/*
 * Devuelve un puntero al material del ultimo objeto agregado, o NULL
 * si la escena esta vacia o el objeto no tiene material.
 */
static Material *material_ultimo(Escena *e)
{
    if (e == NULL ||
        e->num_objetos <= 0 ||
        e->objetos == NULL) {

        return NULL;
    }

    Objeto *ultimo = &e->objetos[e->num_objetos - 1];

    switch (ultimo->tipo) {

        case TIPO_ESFERA:
            return &ultimo->u.esfera.material;

        case TIPO_POLIGONO:
            return &ultimo->u.poligono.material;

        case TIPO_DISCO:
            return &ultimo->u.disco.material;

        case TIPO_ELIPSE:
            return &ultimo->u.elipse.material;
        
        case TIPO_CILINDRO:
            return &ultimo->u.cilindro.material;
        
        case TIPO_CONO:
            return &ultimo->u.cono.material;

        case TIPO_CUADRATICA:
            return &ultimo->u.cuadratica.material;
        
        default:
            return NULL;
    }
}

/* ------------------------------------------------------------------ */
/* Carga de objetos                                                    */
/* ------------------------------------------------------------------ */
static Objeto *reservar_objeto(Escena *escena)
{
    if (escena == NULL) {
        return NULL;
    }

    if (escena->num_objetos >= escena->cap_objetos) {

        int nueva_cap = escena->cap_objetos > 0
            ? escena->cap_objetos * 2
            : ESCENA_CAP_INICIAL;

        Objeto *crecido = realloc(
            escena->objetos,
            (size_t)nueva_cap * sizeof(Objeto)
        );

        if (crecido == NULL) {
            return NULL;
        }

        escena->objetos = crecido;
        escena->cap_objetos = nueva_cap;
    }

    return &escena->objetos[escena->num_objetos];
}

int escena_agregar_esfera(Escena *escena, Vec3 centro, double radio,
                          Color color, Material material)
{
    Objeto *nuevo;
    Objeto *crecido;
    int nueva_cap;

    if (escena == NULL || !material_valido(&material)) {
        return 0;
    }

    if (escena->num_objetos >= escena->cap_objetos) {
        nueva_cap = (escena->cap_objetos > 0) ? escena->cap_objetos * 2
                                              : ESCENA_CAP_INICIAL;
        crecido = (Objeto *)realloc(escena->objetos,
                                    (size_t)nueva_cap * sizeof(Objeto));
        if (crecido == NULL) {
            return 0;
        }
        escena->objetos      = crecido;
        escena->cap_objetos  = nueva_cap;
    }

    nuevo = &escena->objetos[escena->num_objetos];
    nuevo->tipo       = TIPO_ESFERA;
    nuevo->num_planos_corte = 0;
    nuevo->u.esfera.centro = centro;
    nuevo->u.esfera.radio  = radio;
    nuevo->u.esfera.color  = color;
    nuevo->u.esfera.material = material;

    escena->num_objetos++;
    return 1;
}

/*
 * Agrega un poligono a la escena. Devuelve 1 si tuvo exito, 0 si no.
 */

int escena_agregar_poligono(
    Escena *escena,
    const Vec3 *vertices,
    int num_vertices,
    Color color,
    Material material
)
{
    if (escena == NULL || 
        vertices == NULL || 
        num_vertices < 3 || 
        num_vertices > ESCENA_MAX_VERTICES ||
        !material_valido(&material)) {
        return 0;
    }

    /*Crear estructura temporal para validar geometria*/
    Poligono temporal = {
        .vertices = (Vec3 *)vertices,
        .num_vertices = num_vertices,
        .normal = {0.0, 0.0, 0.0},
        .color = color,
        .material = material
    };

    /* Validar aristas */
    if (!validar_aristas(&temporal)) {
        return 0;
    }

    /* Validar coplanaridad*/
    if (!validar_coplanaridad(&temporal)) {
        return 0;
    }

    /* Comprobar que el contorno no se cruce */
    if (!validar_poligono_simple(&temporal)) {
    return 0;
}

    /* Calcular la normal del poligono */
    Vec3 normal;
    if (!calcular_normal_poligono(
        &temporal, 
        &normal)) {
        return 0;
    }

    /* Reservar memorioa para almacenar una copia de los vertices */
    Vec3 *copia = malloc(
        (size_t)num_vertices * sizeof(Vec3)
    );

    if (copia == NULL) {
        return 0;
    }

    for (int i = 0; i < num_vertices; i++) {
        copia[i] = vertices[i];
    }

    /* Aumentar la capacidad del arreglo de objetos si es necesario */
    if (escena->num_objetos >= escena->cap_objetos) {
        int nueva_cap = (escena->cap_objetos > 0) 
        ? escena->cap_objetos * 2
        : ESCENA_CAP_INICIAL;

        Objeto *nuevo = (Objeto *)realloc(
            escena->objetos,
            (size_t)nueva_cap * sizeof(Objeto)
        );

        if (nuevo == NULL) {
            free(copia);
            return 0;
        }

        escena->objetos     = nuevo;
        escena->cap_objetos = nueva_cap;
    }

    /*Agregar el nuevo objeto*/
    Objeto *nuevo = &escena->objetos[escena->num_objetos];
    nuevo->tipo = TIPO_POLIGONO;
    nuevo->num_planos_corte = 0;
    nuevo->u.poligono.vertices = copia;
    nuevo->u.poligono.num_vertices = num_vertices;
    nuevo->u.poligono.normal = normal;
    nuevo->u.poligono.color = color;
    nuevo->u.poligono.material = material;

    escena->num_objetos++;

    return 1;
}

int escena_agregar_disco(
    Escena *escena,
    Vec3 centro,
    Vec3 normal,
    double radio,
    Color color,
    Material material
)
{
    const double EPSILON = 1e-9;

    if (escena == NULL ||
        !material_valido(&material) ||
        !isfinite(radio) ||
        radio <= 0.0 ||
        vec_longitud(normal) <= EPSILON) {

        return 0;
    }

    Objeto *nuevo = reservar_objeto(escena);

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->tipo = TIPO_DISCO;
    nuevo->num_planos_corte = 0;
    nuevo->u.disco.centro = centro;
    nuevo->u.disco.normal = vec_normalizar(normal);
    nuevo->u.disco.radio = radio;
    nuevo->u.disco.color = color;
    nuevo->u.disco.material = material;

    escena->num_objetos++;

    return 1;
}

int escena_agregar_elipse(
    Escena *escena,
    Vec3 foco1,
    Vec3 foco2,
    Vec3 normal,
    double distancia_maxima,
    Color color,
    Material material
)
{
    const double EPSILON = 1e-9;

    if (escena == NULL ||
        !material_valido(&material) ||
        !isfinite(distancia_maxima) ||
        distancia_maxima <= 0.0 ||
        vec_longitud(normal) <= EPSILON) {

        return 0;
    }

    normal = vec_normalizar(normal);

    Vec3 diferencia = vec_restar(foco2, foco1);

    double distancia_focos = vec_longitud(diferencia);

    /* Evitar elipses degeneradas */
    if (distancia_maxima <= distancia_focos + EPSILON) {
        return 0;
    }

    /* Validar coplanaridad */
    if (fabs(vec_punto(normal, diferencia)) > EPSILON) {
        return 0;
    }

    Objeto *nuevo = reservar_objeto(escena);

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->tipo = TIPO_ELIPSE;
    nuevo->num_planos_corte = 0;
    nuevo->u.elipse.foco1 = foco1;
    nuevo->u.elipse.foco2 = foco2;
    nuevo->u.elipse.normal = normal;
    nuevo->u.elipse.distancia_maxima = distancia_maxima;
    nuevo->u.elipse.color = color;
    nuevo->u.elipse.material = material;

    escena->num_objetos++;

    return 1;
}

int escena_agregar_cilindro(
    Escena *escena,
    Vec3 ancla,
    Vec3 eje,
    double radio,
    double d_min,
    double d_max,
    Color color,
    Material material
)
{
    const double EPSILON = 1e-9;

    if (escena == NULL ||
        !material_valido(&material) ||
        !isfinite(radio) ||
        radio <= 0.0 ||
        !isfinite(d_min) ||
        !isfinite(d_max) ||
        d_min >= d_max ||
        vec_longitud(eje) <= EPSILON) {

        return 0;
    }

    Objeto *nuevo = reservar_objeto(escena);

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->tipo = TIPO_CILINDRO;
    nuevo->num_planos_corte = 0;
    nuevo->u.cilindro.ancla = ancla;
    nuevo->u.cilindro.eje = vec_normalizar(eje);
    nuevo->u.cilindro.radio = radio;
    nuevo->u.cilindro.d_min = d_min;
    nuevo->u.cilindro.d_max = d_max;
    nuevo->u.cilindro.color = color;
    nuevo->u.cilindro.material = material;

    escena->num_objetos++;

    return 1;
}

int escena_agregar_cono(
    Escena *escena,
    Vec3 vertice,
    Vec3 eje,
    double m,
    double n,
    double d_min,
    double d_max,
    Color color,
    Material material
)
{
    const double EPSILON = 1e-9;

    if (escena == NULL ||
        !material_valido(&material) ||
        !isfinite(m) ||
        !isfinite(n) ||
        !isfinite(d_min) ||
        !isfinite(d_max) ||
        m <= EPSILON ||
        n <= EPSILON ||
        d_min >= d_max ||
        vec_longitud(eje) <= EPSILON) {

        return 0;
    }

    Objeto *nuevo = reservar_objeto(escena);

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->tipo = TIPO_CONO;
    nuevo->num_planos_corte = 0;
    nuevo->u.cono.vertice = vertice;
    nuevo->u.cono.eje = vec_normalizar(eje);
    nuevo->u.cono.m = m;
    nuevo->u.cono.n = n;
    nuevo->u.cono.d_min = d_min;
    nuevo->u.cono.d_max = d_max;
    nuevo->u.cono.color = color;
    nuevo->u.cono.material = material;

    escena->num_objetos++;

    return 1;
}

int escena_agregar_cuadratica(
    Escena *escena,
    Cuadratica superficie
)
{
    if (escena == NULL ||
        !material_valido(&superficie.material)) {

        return 0;
    }

    double coeficientes[] = {
        superficie.A,
        superficie.B,
        superficie.C,
        superficie.D,
        superficie.E,
        superficie.F,
        superficie.G,
        superficie.H,
        superficie.I,
        superficie.J
    };

    int tiene_termino_variable = 0;

    for (int i = 0; i < 10; i++) {

        if (!isfinite(coeficientes[i])) {
            return 0;
        }

        if (i < 9 && coeficientes[i] != 0.0) {
            tiene_termino_variable = 1;
        }
    }

    /* Rechazar ecuaciones constantes */
    if (!tiene_termino_variable) {
        return 0;
    }

    Objeto *nuevo = reservar_objeto(escena);

    if (nuevo == NULL) {
        return 0;
    }

    *nuevo = (Objeto){0};

    nuevo->tipo = TIPO_CUADRATICA;
    nuevo->u.cuadratica = superficie;

    escena->num_objetos++;

    return 1;
}

int escena_agregar_plano_corte(
    Escena *escena,
    PlanoCorte plano
)
{
    const double EPSILON = 1e-9;

    if (escena == NULL ||
        escena->objetos == NULL ||
        escena->num_objetos <= 0) {

        return 0;
    }

    if (!isfinite(plano.A) ||
        !isfinite(plano.B) ||
        !isfinite(plano.C) ||
        !isfinite(plano.D)) {

        return 0;
    }

    Objeto *ultimo =
        &escena->objetos[escena->num_objetos - 1];

    if (ultimo->num_planos_corte < 0 ||
        ultimo->num_planos_corte >= MAX_PLANOS_CORTE) {

        return 0;
    }

    /* Normalizar los coeficientes del plano */
    double longitud = hypot(
        hypot(plano.A, plano.B),
        plano.C
    );

    if (!isfinite(longitud) ||
        longitud <= EPSILON) {
        return 0;
    }

    plano.A /= longitud;
    plano.B /= longitud;
    plano.C /= longitud;
    plano.D /= longitud;

    /* Guardar el plano en el ultimo objeto */
    ultimo->planos_corte[
        ultimo->num_planos_corte
    ] = plano;

    ultimo->num_planos_corte++;

    return 1;
}
/* =========================================================
   APLICAR MATERIAL AL ULTIMO OBJETO
   ========================================================= */

int escena_aplicar_material(
    Escena *escena,
    Material material
)
{
    if (!material_valido(&material)) {
        return 0;
    }

    Material *destino = material_ultimo(escena);

    if (destino == NULL) {
        return 0;
    }

    *destino = material;

    return 1;
}

/* =========================================================
   AGREGAR FUENTE DE LUZ
   ========================================================= */

int escena_agregar_luz(
    Escena *escena,
    Vec3 posicion,
    double intensidad
)
{
    Luz *nuevo;
    Luz *crecido;
    int nueva_cap;

    /* Validar los parametros */
    if (escena == NULL ||
        !isfinite(intensidad) ||
        intensidad < 0.0 ||
        intensidad > 1.0 ||
        !isfinite(posicion.x) ||
        !isfinite(posicion.y) ||
        !isfinite(posicion.z))
    {
        return 0;
    }

    /* Verificar si se necesita mas espacio */
    if (escena->num_luces >= escena->cap_luces)
    {
        nueva_cap = (escena->cap_luces > 0)
            ? escena->cap_luces * 2
            : 4;

        crecido = (Luz *)realloc(
            escena->luces,
            (size_t)nueva_cap * sizeof(Luz)
        );

        if (crecido == NULL)
        {
            return 0;
        }

        escena->luces = crecido;
        escena->cap_luces = nueva_cap;
    }

    /* Guardar la nueva fuente de luz */
    nuevo = &escena->luces[escena->num_luces];

    nuevo->posicion = posicion;
    nuevo->intensidad = intensidad;

    /* Actualizar la cantidad de luces */
    escena->num_luces++;

    return 1;
}

int escena_num_esferas(const Escena *escena)
{
    int i;
    int cuenta = 0;

    if (escena == NULL) {
        return 0;
    }

    /* Hoy solo hay esferas, pero el conteo recorre por tipo para que siga
     * siendo correcto cuando existan otros objetos. */
    for (i = 0; i < escena->num_objetos; i++) {
        if (escena->objetos[i].tipo == TIPO_ESFERA) {
            cuenta++;
        }
    }
    return cuenta;
}

/* ------------------------------------------------------------------ */
/* Lectura principal                                                   */
/* ------------------------------------------------------------------ */

static int leer_cabecera(Escena *e, char *tokens[], int n, int linea,
                         const char *nombre, int *tiene_resolucion,
                         int *tiene_ojo, int *tiene_ventana, int *tiene_fondo)
{
    double v[4];
    int    i;

    if (strcmp(tokens[0], "resolucion") == 0) {
        if (!arity_ok(e, n, 3, linea, nombre, "resolucion")) return 0;
        if (!a_entero(tokens[1], &e->ancho) ||
            !a_entero(tokens[2], &e->alto)) {
            fijar_error(e, linea, nombre,
                        "\"resolucion\" necesita dos enteros");
            return 0;
        }
        if (e->ancho <= 0 || e->alto <= 0) {
            fijar_error(e, linea, nombre,
                        "la resolucion debe ser mayor que cero");
            return 0;
        }
        *tiene_resolucion = 1;
        return 1;
    }

    if (strcmp(tokens[0], "ojo") == 0) {
        if (!arity_ok(e, n, 4, linea, nombre, "ojo")) return 0;
        for (i = 0; i < 3; i++) {
            if (!a_doble(tokens[1 + i], &v[i])) {
                fijar_error(e, linea, nombre,
                            "\"ojo\" necesita tres coordenadas numericas");
                return 0;
            }
        }
        e->ojo.x = v[0];
        e->ojo.y = v[1];
        e->ojo.z = v[2];
        *tiene_ojo = 1;
        return 1;
    }

    if (strcmp(tokens[0], "ventana") == 0) {
        if (!arity_ok(e, n, 5, linea, nombre, "ventana")) return 0;
        for (i = 0; i < 4; i++) {
            if (!a_doble(tokens[1 + i], &v[i])) {
                fijar_error(e, linea, nombre,
                            "\"ventana\" necesita cuatro coordenadas numericas");
                return 0;
            }
        }
        if (v[0] >= v[2] || v[1] >= v[3]) {
            fijar_error(e, linea, nombre,
                        "la ventana exige xmin < xmax y ymin < ymax");
            return 0;
        }
        e->ventana_xmin = v[0];
        e->ventana_ymin = v[1];
        e->ventana_xmax = v[2];
        e->ventana_ymax = v[3];
        *tiene_ventana = 1;
        return 1;
    }

    if (strcmp(tokens[0], "fondo") == 0) {
        if (!arity_ok(e, n, 4, linea, nombre, "fondo")) return 0;
        for (i = 0; i < 3; i++) {
            if (!a_doble(tokens[1 + i], &v[i]) || v[i] < 0.0 || v[i] > 1.0) {
                fijar_error(e, linea, nombre,
                            "\"fondo\" necesita tres colores en [0, 1]");
                return 0;
            }
        }
        e->fondo.r = v[0];
        e->fondo.g = v[1];
        e->fondo.b = v[2];
        *tiene_fondo = 1;
        return 1;
    }

    if (strcmp(tokens[0], "ambiente") == 0) {
        if (!arity_ok(e, n, 2, linea, nombre, "ambiente")) return 0;
        if (!a_doble(tokens[1], &v[0]) || v[0] < 0.0 || v[0] > 1.0) {
            fijar_error(e, linea, nombre,
                        "\"ambiente\" necesita una intensidad en [0, 1]");
            return 0;
        }
        e->ambiente = v[0];
        return 1;
    }

    fijar_error(e, linea, nombre, "palabra clave desconocida");
    return 0;
}

/*
 * Convierte los cuatro tokens de un material
 * (`ka kd ks kn`) empezando en `inicio`.
 * Devuelve 1 si los valores son validos.
 */
static int a_material(Escena *e, char *tokens[], int inicio, int linea,
                      const char *nombre, const char *clave, Material *m)
{
    double v[4];

    for (int i = 0; i < 4; i++) {
        if (!a_doble(tokens[inicio + i], &v[i])) {
            char motivo[160];
            snprintf(motivo, sizeof(motivo),
                     "\"%s\" necesita cuatro valores numericos "
                     "(ka kd ks kn)", clave);
            fijar_error(e, linea, nombre, motivo);
            return 0;
        }
    }

    m->ka = v[0];
    m->kd = v[1];
    m->ks = v[2];
    m->kn = v[3];

    if (!material_valido(m)) {
        char motivo[160];
        snprintf(motivo, sizeof(motivo),
                 "material invalido en \"%s\": ka, kd y ks en [0, 1] "
                 "y kn mayor que cero", clave);
        fijar_error(e, linea, nombre, motivo);
        return 0;
    }

    return 1;
}

static int leer_disco(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    double v[10];
    Material material;

    if (n != 11 && n != 15) {
        fijar_error(
            e, linea, nombre,
            "cantidad incorrecta de argumentos para disco"
        );

        return 0;
    }

    for (int i = 0; i < 10; i++) {

        if (!a_doble(tokens[i + 1], &v[i])) {

            fijar_error(
                e, linea, nombre,
                "parametro numerico invalido para disco"
            );

            return 0;
        }
    }

    Vec3 centro = {v[0], v[1], v[2]};
    Vec3 normal = {v[3], v[4], v[5]};
    double radio = v[6];

    Color color = {v[7], v[8], v[9]};

    if (color.r < 0.0 || color.r > 1.0 ||
        color.g < 0.0 || color.g > 1.0 ||
        color.b < 0.0 || color.b > 1.0) {

        fijar_error(
            e, linea, nombre,
            "color del disco fuera de rango"
        );

        return 0;
    }

    material_defecto(&material);

    if (n == 15 &&
        !a_material(
            e, tokens, 11, linea,
            nombre, "disco", &material)) {

        return 0;
    }

    if (!escena_agregar_disco(
            e, centro, normal,
            radio, color, material)) {

        fijar_error(
            e, linea, nombre,
            "no se pudo agregar el disco"
        );

        return 0;
    }

    return 1;
}

static int leer_elipse(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    double v[13];
    Material material;

    if (n != 14 && n != 18) {

        fijar_error(
            e, linea, nombre,
            "cantidad incorrecta de argumentos para elipse"
        );

        return 0;
    }

    for (int i = 0; i < 13; i++) {

        if (!a_doble(tokens[i + 1], &v[i])) {

            fijar_error(
                e, linea, nombre,
                "parametro numerico invalido para elipse"
            );

            return 0;
        }
    }

    Vec3 foco1 = {v[0], v[1], v[2]};
    Vec3 foco2 = {v[3], v[4], v[5]};
    Vec3 normal = {v[6], v[7], v[8]};

    double distancia_maxima = v[9];

    Color color = {v[10], v[11], v[12]};

    if (color.r < 0.0 || color.r > 1.0 ||
        color.g < 0.0 || color.g > 1.0 ||
        color.b < 0.0 || color.b > 1.0) {

        fijar_error(
            e, linea, nombre,
            "color de la elipse fuera de rango"
        );

        return 0;
    }

    material_defecto(&material);

    if (n == 18 &&
        !a_material(
            e, tokens, 14, linea,
            nombre, "elipse", &material)) {

        return 0;
    }

    if (!escena_agregar_elipse(
            e, foco1, foco2,
            normal, distancia_maxima,
            color, material)) {

        fijar_error(
            e, linea, nombre,
            "no se pudo agregar la elipse"
        );

        return 0;
    }

    return 1;
}

static int leer_cilindro(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    double v[12];
    Material material;

    if (n != 13 && n != 17) {
        fijar_error(
            e, linea, nombre,
            "cantidad incorrecta de argumentos para cilindro"
        );

        return 0;
    }

    for (int i = 0; i < 12; i++) {

        if (!a_doble(tokens[i + 1], &v[i])) {

            fijar_error(
                e, linea, nombre,
                "parametro numerico invalido para cilindro"
            );

            return 0;
        }
    }

    Vec3 ancla = {v[0], v[1], v[2]};
    Vec3 eje = {v[3], v[4], v[5]};

    double radio = v[6];
    double d_min = v[7];
    double d_max = v[8];

    Color color = {v[9], v[10], v[11]};

    if (color.r < 0.0 || color.r > 1.0 ||
        color.g < 0.0 || color.g > 1.0 ||
        color.b < 0.0 || color.b > 1.0) {

        fijar_error(
            e, linea, nombre,
            "color del cilindro fuera de rango"
        );

        return 0;
    }

    material_defecto(&material);

    if (n == 17 &&
        !a_material(
            e, tokens, 13, linea,
            nombre, "cilindro", &material)) {

        return 0;
    }

    if (!escena_agregar_cilindro(
            e, ancla, eje,
            radio, d_min, d_max,
            color, material)) {

        fijar_error(
            e, linea, nombre,
            "no se pudo agregar el cilindro"
        );

        return 0;
    }

    return 1;
}

static int leer_cono(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    double v[13];
    Material material;

    if (n != 14 && n != 18) {

        fijar_error(
            e, linea, nombre,
            "cantidad incorrecta de argumentos para cono"
        );

        return 0;
    }

    for (int i = 0; i < 13; i++) {

        if (!a_doble(tokens[i + 1], &v[i])) {

            fijar_error(
                e, linea, nombre,
                "parametro numerico invalido para cono"
            );

            return 0;
        }
    }

    Vec3 vertice = {v[0], v[1], v[2]};
    Vec3 eje = {v[3], v[4], v[5]};

    double m = v[6];
    double razon_n = v[7];

    double d_min = v[8];
    double d_max = v[9];

    Color color = {v[10], v[11], v[12]};

    if (color.r < 0.0 || color.r > 1.0 ||
        color.g < 0.0 || color.g > 1.0 ||
        color.b < 0.0 || color.b > 1.0) {

        fijar_error(
            e, linea, nombre,
            "color del cono fuera de rango"
        );

        return 0;
    }

    material_defecto(&material);

    if (n == 18 &&
        !a_material(
            e, tokens, 14, linea,
            nombre, "cono", &material)) {

        return 0;
    }

    if (!escena_agregar_cono(
            e, vertice, eje,
            m, razon_n, d_min, d_max,
            color, material)) {

        fijar_error(
            e, linea, nombre,
            "no se pudo agregar el cono"
        );

        return 0;
    }

    return 1;
}

static int leer_cuadratica(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    double v[13];

    if (n != 14 && n != 18) {

        fijar_error(
            e, linea, nombre,
            "cantidad incorrecta de argumentos para cuadratica"
        );

        return 0;
    }

    for (int i = 0; i < 13; i++) {

        if (!a_doble(tokens[i + 1], &v[i])) {

            fijar_error(
                e, linea, nombre,
                "parametro numerico invalido para cuadratica"
            );

            return 0;
        }
    }

    Cuadratica superficie = {0};

    superficie.A = v[0];
    superficie.B = v[1];
    superficie.C = v[2];
    superficie.D = v[3];
    superficie.E = v[4];
    superficie.F = v[5];
    superficie.G = v[6];
    superficie.H = v[7];
    superficie.I = v[8];
    superficie.J = v[9];

    superficie.color = (Color){
        v[10], v[11], v[12]
    };

    if (v[10] < 0.0 || v[10] > 1.0 ||
        v[11] < 0.0 || v[11] > 1.0 ||
        v[12] < 0.0 || v[12] > 1.0) {

        fijar_error(
            e, linea, nombre,
            "color de la cuadratica fuera de rango"
        );

        return 0;
    }

    material_defecto(&superficie.material);

    if (n == 18 &&
        !a_material(
            e, tokens, 14, linea,
            nombre, "cuadratica",
            &superficie.material)) {

        return 0;
    }

    if (!escena_agregar_cuadratica(
            e, superficie)) {

        fijar_error(
            e, linea, nombre,
            "no se pudo agregar la cuadratica"
        );

        return 0;
    }

    return 1;
}

static int leer_corte(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    double v[4];

    if (n != 5) {

        fijar_error(
            e, linea, nombre,
            "corte espera cuatro coeficientes"
        );

        return 0;
    }

    for (int i = 0; i < 4; i++) {

        if (!a_doble(tokens[i + 1], &v[i])) {

            fijar_error(
                e, linea, nombre,
                "coeficiente invalido para plano de corte"
            );

            return 0;
        }
    }

    PlanoCorte plano = {
        v[0], v[1], v[2], v[3]
    };

    if (!escena_agregar_plano_corte(e, plano)) {

        fijar_error(
            e, linea, nombre,
            "plano de corte invalido o sin objeto previo"
        );

        return 0;
    }

    return 1;
}

static int leer_esfera(Escena *e, char *tokens[], int n, int linea,
                       const char *nombre)
{
    double v[7];
    Vec3   centro;
    Color  color;
    Material material;
    int    i;

    /*
     * esfera cx cy cz radio r g b            -> 8 tokens
     * esfera cx cy cz radio r g b ka kd ks kn -> 12 tokens
     */
    if (n != 8 && n != 12) {
        fijar_error(e, linea, nombre,
                    "\"esfera\" espera 7 argumentos "
                    "(o 11 con material: ka kd ks kn)");
        return 0;
    }

    for (i = 0; i < 7; i++) {
        if (!a_doble(tokens[1 + i], &v[i])) {
            fijar_error(e, linea, nombre,
                        "\"esfera\" necesita siete valores numericos");
            return 0;
        }
    }

    if (v[3] <= 0.0) {
        fijar_error(e, linea, nombre, "el radio debe ser mayor que cero");
        return 0;
    }
    if (v[4] < 0.0 || v[4] > 1.0 ||
        v[5] < 0.0 || v[5] > 1.0 ||
        v[6] < 0.0 || v[6] > 1.0) {
        fijar_error(e, linea, nombre,
                    "el color de la esfera debe estar en [0, 1]");
        return 0;
    }

    material_defecto(&material);

    if (n == 12 && !a_material(e, tokens, 8, linea, nombre, "esfera", &material)) {
        return 0;
    }

    centro.x = v[0];
    centro.y = v[1];
    centro.z = v[2];
    color.r  = v[4];
    color.g  = v[5];
    color.b  = v[6];

    if (!escena_agregar_esfera(e, centro, v[3], color, material)) {
        fijar_error(e, linea, nombre, "no hay memoria para mas objetos");
        return 0;
    }
    return 1;
}

/* =========================================================
   LECTURA DE POLIGONOS
   ========================================================= 
   Formato: poligono <num_vertices> <x1> <y1> <z1> ... <xn> <yn> <zn> <r> <g> <b>
   - num_vertices: numero de vertices del poligono (>= 3)
   - <xi> <yi> <zi>: coordenadas de cada vertice
   - <r> <g> <b>: color del poligono en el rango [0, 1]
   - Todos los valores son decimales (formato C, admite notacion 1e-3).
   */
   static int leer_poligono(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    int cantidad;

    /*Leer cantidad de verticaes*/
    if (n < 2 || !a_entero(tokens[1], &cantidad)) {
        fijar_error(
            e,
            linea,
            nombre,
            "\"poligono\" necesita un numero de vertices valido"
        );
        return 0;
    }
    
    if (cantidad < 3 || 
    cantidad > ESCENA_MAX_VERTICES) {
        fijar_error(
            e,
            linea,
            nombre,
            "\"poligono\" necesita al menos 3 vertices y como maximo 16"
        );
        return 0;
    }

    /*Formato:
    1 token: poligono
    1 token: cantidad de vertices
    3*N tokens: coordenadas de cada vertice
    3 tokens: color del poligono
    4 tokens opcionales: material (ka kd ks kn)
    */
    int esperados = 2 + 3 * cantidad + 3;

    if (n != esperados && n != esperados + 4) {
        fijar_error(
            e,
            linea,
            nombre,
            "cantidad incorrecta de coordenadas para el poligono "
            "(color, o color mas material ka kd ks kn)"
        );
        return 0;
    }

    Vec3 vertices[ESCENA_MAX_VERTICES];

    /* Leer las coordenadas de los vertices */
    for (int i = 0; i < cantidad; i++) {

        int indice = 2 + 3 * i;

        if (!a_doble(tokens[indice], &vertices[i].x) ||
            !a_doble(tokens[indice + 1], &vertices[i].y) ||
            !a_doble(tokens[indice + 2], &vertices[i].z)) {
            fijar_error(
                e,
                linea,
                nombre,
                "coordenadas de vertice invalidas"
            );
            return 0;
        }
    }

    /* Leer el color del poligono */
    int indice_color = 2 + 3 * cantidad;
    Color color;

    if (!a_doble(tokens[indice_color], &color.r) ||
        !a_doble(tokens[indice_color + 1], &color.g) ||
        !a_doble(tokens[indice_color + 2], &color.b)) {
        fijar_error(
            e,
            linea,
            nombre,
            "color del poligono invalido"
        );
        return 0;
    }

    if (color.r < 0.0 || color.r > 1.0 ||
        color.g < 0.0 || color.g > 1.0 ||
        color.b < 0.0 || color.b > 1.0) {
        fijar_error(
            e,
            linea,
            nombre,
            "color del poligono debe estar en el rango [0, 1]"
        );
        return 0;
    }

    /* Material opcional al final de la linea */
    Material material;
    material_defecto(&material);

    if (n == esperados + 4 &&
        !a_material(e, tokens, esperados, linea, nombre, "poligono", &material)) {
        return 0;
    }

    /* Validar y agregar el poligono a la escena */
    if (!escena_agregar_poligono(
        e,
        vertices,
        cantidad,
        color,
        material)) {
        
        fijar_error(
            e,
            linea,
            nombre,
            "no se pudo agregar el poligono a la escena"
        );
        return 0;
        }

    return 1;
}

/* =========================================================
   LECTURA DE FUENTES DE LUZ
   ========================================================= */

static int leer_luz(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    double valores[4];
    Vec3 posicion;

    /* luz x y z intensidad = 5 tokens */
    if (!arity_ok(e, n, 5, linea, nombre, "luz"))
    {
        return 0;
    }

    /* Convertir los cuatro valores numericos */
    for (int i = 0; i < 4; i++)
    {
        if (!a_doble(tokens[i + 1], &valores[i]))
        {
            fijar_error(
                e,
                linea,
                nombre,
                "\"luz\" necesita cuatro valores numericos"
            );
            return 0;
        }
    }

    /* Validar intensidad */
    if (valores[3] < 0.0 || valores[3] > 1.0)
    {
        fijar_error(
            e,
            linea,
            nombre,
            "la intensidad de la luz debe estar en [0, 1]"
        );
        return 0;
    }

    /* Establecer la posicion */
    posicion.x = valores[0];
    posicion.y = valores[1];
    posicion.z = valores[2];

    /* Agregar la luz a la escena */
    if (!escena_agregar_luz(e, posicion, valores[3]))
    {
        fijar_error(
            e,
            linea,
            nombre,
            "no se pudo agregar la fuente de luz"
        );
        return 0;
    }

    return 1;
}


/* =========================================================
   LECTURA DE MATERIALES
   =========================================================
   Formato: material <ka> <kd> <ks> <kn>
            material <ka> <kd> <ks> <kn> <reflejo> <transparencia>
   Se coloca despues del objeto al que debe aplicarse.
   Con cuatro valores se conservan el reflejo y la transparencia
   que el objeto ya tuviera (no se reinician).
*/

static int leer_material(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre
)
{
    Material *actual;
    Material material;
    double   v[2];

    if (n != 5 && n != 7) {
        fijar_error(
            e,
            linea,
            nombre,
            "\"material\" espera 4 argumentos (ka kd ks kn) "
            "o 6 (mas reflejo transparencia)"
        );
        return 0;
    }

    actual = material_ultimo(e);
    if (actual == NULL) {
        fijar_error(
            e,
            linea,
            nombre,
            "\"material\" debe ir despues de un objeto"
        );
        return 0;
    }

    /* Parte del material actual para no borrar reflejo/transparencia. */
    material = *actual;

    if (!a_material(e, tokens, 1, linea, nombre, "material", &material)) {
        return 0;
    }

    if (n == 7) {
        if (!a_doble(tokens[5], &v[0]) || !a_doble(tokens[6], &v[1])) {
            fijar_error(
                e,
                linea,
                nombre,
                "\"material\" necesita reflejo y transparencia numericos "
                "en [0, 1]"
            );
            return 0;
        }
        material.reflejo = v[0];
        material.transparencia = v[1];

        if (!material_valido(&material)) {
            fijar_error(
                e,
                linea,
                nombre,
                "reflejo y transparencia deben estar en [0, 1]"
            );
            return 0;
        }
    }

    if (!escena_aplicar_material(e, material)) {
        fijar_error(
            e,
            linea,
            nombre,
            "no se pudo aplicar el material"
        );
        return 0;
    }

    return 1;
}

/* =========================================================
   LECTURA DE PROPIEDADES OPTICAS DEL ULTIMO OBJETO
   =========================================================
   Formato: espejo <reflejo>            (0 = opaco, 1 = espejo)
            transparencia <transparencia>  (0 = opaco, 1 = transparente)
   Se colocan despues del objeto al que aplican, como "material".
*/

static int leer_propiedad_optica(
    Escena *e,
    char *tokens[],
    int n,
    int linea,
    const char *nombre,
    int es_reflejo
)
{
    const char *clave = es_reflejo ? "espejo" : "transparencia";
    Material   *material;
    double      valor;

    if (!arity_ok(e, n, 2, linea, nombre, clave)) {
        return 0;
    }

    if (!a_doble(tokens[1], &valor) || valor < 0.0 || valor > 1.0) {
        char motivo[160];
        snprintf(motivo, sizeof(motivo),
                 "\"%s\" necesita un valor en [0, 1]", clave);
        fijar_error(e, linea, nombre, motivo);
        return 0;
    }

    material = material_ultimo(e);
    if (material == NULL) {
        char motivo[160];
        snprintf(motivo, sizeof(motivo),
                 "\"%s\" debe ir despues de un objeto", clave);
        fijar_error(e, linea, nombre, motivo);
        return 0;
    }

    if (es_reflejo) {
        material->reflejo = valor;
    } else {
        material->transparencia = valor;
    }

    return 1;
}


/* =========================================================
   LECTURA DE LA ESCENA
   ========================================================= */

int escena_leer_flujo(Escena *escena, FILE *flujo, const char *nombre)
{
    char  linea[MAX_LINEA];
    char *tokens[MAX_TOKENS];
    int   n;
    int   num_linea = 0;
    int   num_esferas = 0;
    int   tiene_resolucion = 0;
    int   tiene_ojo = 0;
    int   tiene_ventana = 0;
    int   tiene_fondo = 0;
    int   termino = 0;
    size_t largo;
    char *comentario;

    if (escena == NULL) {
        return 0;
    }
    escena_inicializar(escena);

    if (flujo == NULL) {
        fijar_error(escena, 0, nombre, "no se pudo abrir el archivo");
        return 0;
    }
    if (escena->objetos == NULL) {
        fijar_error(escena, 0, nombre, "sin memoria para la escena");
        return 0;
}

    while (fgets(linea, (int)sizeof(linea), flujo) != NULL) {
        num_linea++;

        /* Linea demasiado larga: evita lecturas truncadas silenciosas. */
        largo = strlen(linea);
        if (largo == sizeof(linea) - 1 && linea[largo - 1] != '\n') {
            fijar_error(escena, num_linea, nombre, "linea demasiado larga");
            return 0;
        }

        /* Corta comentarios y lineas vacias. */
        comentario = strchr(linea, '#');
        if (comentario != NULL) {
            *comentario = '\0';
        }

        n = tokenizar(linea, tokens);
        if (n < 0) {
            fijar_error(escena, num_linea, nombre, "demasiados tokens");
            return 0;
        }
        if (n == 0) {
            continue;
        }

        if (strcmp(tokens[0], "fin") == 0) {
            if (n != 1) {
                fijar_error(escena, num_linea, nombre,
                            "\"fin\" no admite argumentos");
                return 0;
            }
            termino = 1;
            break;
        }

        if (strcmp(tokens[0], "esfera") == 0) {
            if (!leer_esfera(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            num_esferas++;
            continue;
        }

        /* Leer un poligono */
        if (strcmp(tokens[0], "poligono") == 0)
        {
            if (!leer_poligono(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            continue;
        }

        if (strcmp(tokens[0], "disco") == 0) {
            if (!leer_disco(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            continue;
        }
        
        if (strcmp(tokens[0], "elipse") == 0) {
            if (!leer_elipse(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
        continue;
        }

        if (strcmp(tokens[0], "cilindro") == 0) {
            if (!leer_cilindro(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            continue;
        }

        if (strcmp(tokens[0], "cono") == 0) {
            if (!leer_cono(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            continue;
        }

        if (strcmp(tokens[0], "cuadratica") == 0) {
            if (!leer_cuadratica(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            continue;
        }

        if (strcmp(tokens[0], "corte") == 0) {
            if (!leer_corte(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            continue;
        }

        /* Leer una fuente de luz */
        if (strcmp(tokens[0], "luz") == 0)
        {
            if (!leer_luz(escena, tokens, n, num_linea, nombre)){
                return 0;
            }
            continue;
        }

        /* Asignar material al ultimo objeto leido */
        if (strcmp(tokens[0], "material") == 0)
        {
            if (!leer_material(escena, tokens, n, num_linea, nombre)) {
                return 0;
            }
            continue;
        }

        /* Convertir el ultimo objeto en espejo */
        if (strcmp(tokens[0], "espejo") == 0)
        {
            if (!leer_propiedad_optica(escena, tokens, n, num_linea, nombre, 1)) {
                return 0;
            }
            continue;
        }

        /* Hacer transparente el ultimo objeto */
        if (strcmp(tokens[0], "transparencia") == 0)
        {
            if (!leer_propiedad_optica(escena, tokens, n, num_linea, nombre, 0)) {
                return 0;
            }
            continue;
        }

        if (!leer_cabecera(escena, tokens, n, num_linea, nombre,
                        &tiene_resolucion, &tiene_ojo,
                        &tiene_ventana, &tiene_fondo)) {
            return 0;
        }
    }

    if (ferror(flujo)) {
        fijar_error(escena, 0, nombre, "error al leer el archivo");
        return 0;
    }
    
    if (!termino && feof(flujo) == 0) {
        fijar_error(escena, 0, nombre, "fin de archivo inesperado");
        return 0;
    }
    if (!tiene_resolucion) {
        fijar_error(escena, 0, nombre, "falta la linea \"resolucion\"");
        return 0;
    }
    if (!tiene_ojo) {
        fijar_error(escena, 0, nombre, "falta la linea \"ojo\"");
        return 0;
    }
    if (!tiene_ventana) {
        fijar_error(escena, 0, nombre, "falta la linea \"ventana\"");
        return 0;
    }
    if (!tiene_fondo) {
        fijar_error(escena, 0, nombre, "falta la linea \"fondo\"");
        return 0;
    }
    if (num_esferas < ESCENA_MIN_ESFERAS) {
        char motivo[160];
        snprintf(motivo, sizeof(motivo),
                 "se requieren al menos %d esferas, hay %d",
                 ESCENA_MIN_ESFERAS, num_esferas);
        fijar_error(escena, 0, nombre, motivo);
        return 0;
    }

    return 1;
}

int escena_leer(Escena *escena, const char *ruta)
{
    FILE *flujo;
    int   ok;

    if (escena == NULL || ruta == NULL) {
        return 0;
    }

    flujo = fopen(ruta, "r");
    if (flujo == NULL) {
        escena_inicializar(escena);
        snprintf(escena->error, sizeof(escena->error),
                 "%s: no se pudo abrir el archivo", ruta);
        return 0;
    }

    ok = escena_leer_flujo(escena, flujo, ruta);
    fclose(flujo);
    return ok;
}

/* ------------------------------------------------------------------ */
/* Liberacion                                                          */
/* ------------------------------------------------------------------ */

/* =========================================================
   LIBERAR MEMORIA DE LA ESCENA
   ========================================================= */

void escena_liberar(Escena *escena)
{
    if (escena == NULL)
    {
        return;
    }

    /*
     * Liberar la memoria de los vertices
     * de cada poligono.
     */
    for (int i = 0; i < escena->num_objetos; i++)
    {
        Objeto *objeto = &escena->objetos[i];

        if (objeto->tipo == TIPO_POLIGONO)
        {
            free(objeto->u.poligono.vertices);

            objeto->u.poligono.vertices = NULL;
            objeto->u.poligono.num_vertices = 0;
        }
    }

    /* Liberar arreglo de objetos */
    free(escena->objetos);

    escena->objetos = NULL;
    escena->num_objetos = 0;
    escena->cap_objetos = 0;

    free(escena->luces);

    escena->luces = NULL;
    escena->num_luces = 0;
    escena->cap_luces = 0;
}
