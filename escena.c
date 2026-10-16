/*
 * escena.c
 *
 * Lectura y validacion del archivo de descripcion de la escena.
 * No imprime nada por stdout/stderr: los errores se guardan en la escena y
 * los decide imprimir (o no) quien llame a escena_leer().
 */
#include "escena.h"

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

    escena->cap_objetos = ESCENA_CAP_INICIAL;
    escena->objetos = (Objeto *)calloc((size_t)escena->cap_objetos,
                                       sizeof(Objeto));
    
    /* Inicializar el arreglo dinamico de luces */
    escena->cap_luces = 4;
    escena->num_luces = 0;

    escena->luces = (Luz *)calloc(
        (size_t)escena->cap_luces,
        sizeof(Luz)
    );
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
/* Carga de objetos                                                    */
/* ------------------------------------------------------------------ */

int escena_agregar_esfera(Escena *escena, Vec3 centro, double radio,
                          Color color)
{
    Objeto *nuevo;
    Objeto *crecido;
    int nueva_cap;

    if (escena == NULL) {
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
    nuevo->u.esfera.centro = centro;
    nuevo->u.esfera.radio  = radio;
    nuevo->u.esfera.color  = color;

    escena->num_objetos++;
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

        /* Ampliar el arreglo */
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

    fijar_error(e, linea, nombre, "palabra clave desconocida");
    return 0;
}

static int leer_esfera(Escena *e, char *tokens[], int n, int linea,
                       const char *nombre)
{
    double v[7];
    Vec3   centro;
    Color  color;
    int    i;

    if (!arity_ok(e, n, 8, linea, nombre, "esfera")) return 0;

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

    centro.x = v[0];
    centro.y = v[1];
    centro.z = v[2];
    color.r  = v[4];
    color.g  = v[5];
    color.b  = v[6];

    if (!escena_agregar_esfera(e, centro, v[3], color)) {
        fijar_error(e, linea, nombre, "no hay memoria para mas objetos");
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
    if (escena->objetos == NULL || escena->luces == NULL) {
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

        /* Leer una fuente de luz */
        if (strcmp(tokens[0], "luz") == 0)
        {
            if (!leer_luz(escena, tokens, n, num_linea, nombre)){
                return 0;
            }
            continue;
        }

        if (!leer_cabecera(escena, tokens, n, num_linea, nombre,
                           &tiene_resolucion, &tiene_ojo, &tiene_ventana,
                           &tiene_fondo)) {
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

    /* Liberar objetos */
    free(escena->objetos);

    escena->objetos = NULL;
    escena->num_objetos = 0;
    escena->cap_objetos = 0;

    /* Liberar fuentes de luz */
    free(escena->luces);

    escena->luces = NULL;
    escena->num_luces = 0;
    escena->cap_luces = 0;
}
