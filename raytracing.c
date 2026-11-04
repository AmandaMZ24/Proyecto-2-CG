#include <float.h>
#include <math.h>
#include <stddef.h>

#include "raytracing.h"
#include "escena.h"
#include "vectores.h"
#include "poligonos.h"

/*
 * Desplazamiento del origen de los rayos de sombra y de los rayos
 * reflejados respecto a la superficie. Evita la auto-interseccion
 * (self-shadowing) por errores de punto flotante.
 */
#define EPSILON_SUPERFICIE 1e-4

/* Calcula un punto sobre el rayo: P(t) = origen + t * direccion */
static Vec3 punto_en_rayo(Vec3 origen, Vec3 direccion, double t)
{
    return vec_sumar(origen, vec_escalar(direccion, t));
}

int intersecar_esfera(Vec3 origen, Vec3 direccion, const Esfera *esfera, double *t)
{
    if (!esfera || !t || esfera->radio <= 0.0) return 0;

    Vec3 oc = vec_restar(origen, esfera->centro);

    double a = vec_punto(direccion, direccion);
    if (a <= 0.0) return 0;

    double b = 2.0 * vec_punto(oc, direccion);
    double c = vec_punto(oc, oc) - esfera->radio * esfera->radio;

    double discriminante = b * b - 4.0 * a * c;
    if (discriminante < 0.0) return 0;

    double raiz = sqrt(discriminante);
    double t1 = (-b - raiz) / (2.0 * a);
    double t2 = (-b + raiz) / (2.0 * a);

    const double epsilon = 1e-9;

    if (t1 > epsilon) {
        *t = t1;
        return 1;
    }

    if (t2 > epsilon) {
        *t = t2;
        return 1;
    }

    return 0;
}

/*
 * Interseccion de un rayo con un objeto cualquiera de la escena.
 * Devuelve 1 si hay impacto hacia adelante y escribe t.
 */
static int intersecar_objeto(const Objeto *objeto, Vec3 origen,
                             Vec3 direccion, double *t)
{
    if (objeto == NULL || t == NULL) {
        return 0;
    }

    if (objeto->tipo == TIPO_ESFERA) {
        return intersecar_esfera(origen, direccion, &objeto->u.esfera, t);
    }

    if (objeto->tipo == TIPO_POLIGONO) {
        return intersecar_poligono(origen, direccion, &objeto->u.poligono, t);
    }

    return 0;
}

/* ================================================================== */
/*  FIRST_INTERSECTION: objeto mas cercano golpeado por un rayo        */
/* ================================================================== */

int First_Intersection(const Escena *escena, Vec3 origen, Vec3 direccion,
                       int *indice, double *t)
{
    double mejor_t = DBL_MAX;
    int    mejor_i = -1;

    if (escena == NULL || escena->objetos == NULL ||
        escena->num_objetos <= 0) {
        return 0;
    }

    for (int i = 0; i < escena->num_objetos; ++i)
    {
        double t_actual = 0.0;

        if (intersecar_objeto(&escena->objetos[i], origen, direccion,
                              &t_actual) &&
            t_actual > 1e-9 &&
            t_actual < mejor_t)
        {
            mejor_t = t_actual;
            mejor_i = i;
        }
    }

    if (mejor_i < 0) {
        return 0;
    }

    if (indice != NULL) {
        *indice = mejor_i;
    }
    if (t != NULL) {
        *t = mejor_t;
    }

    return 1;
}

/* ================================================================== */
/*  Datos geometricos del punto de impacto                             */
/* ================================================================== */

/*
 * Obtiene la normal, el color base y el material del objeto golpeado.
 * La normal siempre queda orientada hacia el lado del que llega el rayo.
 */
static void datos_de_impacto(const Objeto *objeto, Vec3 punto, Vec3 direccion,
                             Vec3 *normal, Color *color, Material *material)
{
    *normal = (Vec3){0.0, 0.0, 0.0};
    *color  = (Color){0.0, 0.0, 0.0};
    material_defecto(material);

    if (objeto == NULL) {
        return;
    }

    if (objeto->tipo == TIPO_ESFERA)
    {
        const Esfera *esfera = &objeto->u.esfera;

        /* Vector desde el centro hasta el punto de impacto */
        *normal = vec_normalizar(vec_restar(punto, esfera->centro));
        *color  = esfera->color;
        *material = esfera->material;
    }
    else if (objeto->tipo == TIPO_POLIGONO)
    {
        const Poligono *poligono = &objeto->u.poligono;

        /* Normal calculada al cargar el poligono */
        *normal = poligono->normal;
        *color  = poligono->color;
        *material = poligono->material;

        if (vec_longitud(*normal) <= 1e-12) {
            Vec3 n = {0.0, 0.0, 0.0};
            if (calcular_normal_poligono(poligono, &n)) {
                *normal = n;
            }
        }
    }

    /*
     * Orientar la normal hacia el lado desde el cual llega el rayo.
     * Para una esfera vista desde afuera esto no cambia nada; solo
     * actua cuando el observador queda dentro del objeto.
     */
    if (vec_punto(*normal, direccion) > 0.0)
    {
        *normal = vec_escalar(*normal, -1.0);
    }
}

/* ================================================================== */
/*  SOMBRAS                                                            */
/* ================================================================== */

/*
 * Devuelve 1 si hay un objeto entre el punto y la fuente de luz.
 *
 * Se lanza un rayo de sombra desde un punto ligeramente desplazado
 * hacia afuera a lo largo de la normal, en la direccion de la luz.
 * Si la interseccion mas cercana esta antes que la luz, el punto
 * queda en sombra.
 */
static int hay_sombra(const Escena *escena, Vec3 punto, Vec3 normal,
                      Vec3 direccion_luz, double distancia_luz)
{
    Vec3 origen = vec_sumar(punto, vec_escalar(normal, EPSILON_SUPERFICIE));

    int    indice = -1;
    double t = 0.0;

    if (!First_Intersection(escena, origen, direccion_luz, &indice, &t)) {
        return 0;   /* nada obstruye el camino hacia la luz */
    }

    /*
     * El obstaculo debe estar entre el punto y la luz, con un margen
     * para no sombrear por errores de punto flotante.
     */
    return t < distancia_luz - EPSILON_SUPERFICIE;
}

/* ================================================================== */
/*  DE_QUE_COLOR: modelo de iluminacion completo                       */
/* ================================================================== */

Color De_Que_Color(const Escena *escena, Vec3 origen, Vec3 direccion)
{
    Color negro = {0.0, 0.0, 0.0};

    if (escena == NULL) {
        return negro;
    }

    int    indice = -1;
    double mejor_t = 0.0;

    /* 1) Superficie mas cercana golpeada por el rayo */
    if (!First_Intersection(escena, origen, direccion, &indice, &mejor_t)) {
        return escena->fondo;
    }

    const Objeto *objeto = &escena->objetos[indice];

    /* 2) Geometria y material del impacto */
    Vec3    punto   = punto_en_rayo(origen, direccion, mejor_t);
    Vec3    normal  = {0.0, 0.0, 0.0};
    Color   base    = {0.0, 0.0, 0.0};
    Material material;
    material_defecto(&material);

    datos_de_impacto(objeto, punto, direccion, &normal, &base, &material);

    if (vec_longitud(normal) <= 0.0) {
        return negro;
    }

    /* Vector unitario hacia el observador (el ojo de la escena) */
    Vec3 V = vec_normalizar(vec_restar(escena->ojo, punto));

    /*
     * 3) Componente ambiental: ka * Ia.
     * No depende de la normal ni de la luz, por lo que no recibe sombra.
     */
    Color color = {
        base.r * material.ka * escena->ambiente,
        base.g * material.ka * escena->ambiente,
        base.b * material.ka * escena->ambiente
    };

    if (escena->luces == NULL || escena->num_luces <= 0) {
        return color;
    }

    /* 4) Recorrer todas las fuentes de luz de la escena */
    for (int k = 0; k < escena->num_luces; ++k)
    {
        Vec3 posicion = escena->luces[k].posicion;
        double Ip     = escena->luces[k].intensidad;

        /* Vector desde la interseccion hacia la luz */
        Vec3 vector_luz = vec_restar(posicion, punto);

        /* Distancia hacia la fuente */
        double distancia_luz = vec_longitud(vector_luz);

        if (distancia_luz <= 0.0) {
            continue;
        }

        /* Vector unitario hacia la luz */
        Vec3 L = vec_normalizar(vector_luz);

        /* Ley de Lambert: coseno entre la normal y la luz */
        double NdotL = vec_punto(normal, L);

        if (NdotL <= 0.0) {
            /* La superficie mira hacia el lado opuesto de la luz */
            continue;
        }

        /*
         * 5) Sombra: si algo obstruye el camino hasta la luz,
         *    esta fuente no aporta difusa ni especular.
         */
        if (hay_sombra(escena, punto, normal, L, distancia_luz)) {
            continue;
        }

        /* Atenuacion por distancia: Fatt = 1 / (C1 + C2*d + C3*d^2) */
        double C1 = 1.0;
        double C2 = 0.02;
        double C3 = 0.004;

        double denominador = C1 + C2 * distancia_luz + C3 * distancia_luz * distancia_luz;
        double Fatt = fmin(1.0, 1.0 / denominador);

        /* --- Componente difusa: kd * Ip * Fatt * (N . L) --- */
        double difusa = material.kd * Ip * Fatt * NdotL;

        color.r += base.r * difusa;
        color.g += base.g * difusa;
        color.b += base.b * difusa;

        /*
         * --- Componente especular: ks * Ip * Fatt * (R . V)^kn ---
         *
         * R es la reflexion de L respecto de N:
         *     R = 2 * (N . L) * N - L
         * y V el vector hacia el observador. El exponente kn
         * controla que tan concentrado queda el brillo.
         */
        Vec3 R = vec_restar(vec_escalar(normal, 2.0 * NdotL), L);

        double RdotV = vec_punto(R, V);

        if (RdotV > 0.0)
        {
            double especular = material.ks * Ip * Fatt * pow(RdotV, material.kn);

            color.r += especular;
            color.g += especular;
            color.b += especular;
        }
    }

    /* 5) Limitar cada canal al rango [0, 1] */
    color.r = fmin(1.0, fmax(0.0, color.r));
    color.g = fmin(1.0, fmax(0.0, color.g));
    color.b = fmin(1.0, fmax(0.0, color.b));

    return color;
}

/* ================================================================== */
/*  TRAZAR_PIXEL                                                       */
/* ================================================================== */

Color trazar_pixel(const Escena *escena, int columna, int fila)
{
    Color negro = {0.0, 0.0, 0.0};

    if (!escena || escena->ancho <= 0 || escena->alto <= 0 ||
        columna < 0 || columna >= escena->ancho ||
        fila < 0 || fila >= escena->alto) {
        return negro;
    }

    /* El centro del pixel se proyecta en el plano de imagen z = 0 */
    double x = escena->ventana_xmin +
        ((double)columna + 0.5) / escena->ancho *
        (escena->ventana_xmax - escena->ventana_xmin);

    double y = escena->ventana_ymax -
        ((double)fila + 0.5) / escena->alto *
        (escena->ventana_ymax - escena->ventana_ymin);

    Vec3 punto_ventana = {x, y, 0.0};
    Vec3 direccion = vec_restar(punto_ventana, escena->ojo);

    return De_Que_Color(escena, escena->ojo, direccion);
}
