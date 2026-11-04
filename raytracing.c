#include <float.h>
#include <math.h>
#include <stddef.h>

#include "raytracing.h"
#include "vectores.h"
#include "poligonos.h"

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

    /* Buscar la interseccion mas cercana */
double mejor_t = DBL_MAX;
Color color = escena->fondo;

/* Ahora guardamos cualquier tipo de objeto */
const Objeto *objeto_golpeado = NULL;

for (int i = 0; i < escena->num_objetos; ++i)
{
    const Objeto *objeto = &escena->objetos[i];

    double t = 0.0;
    int hay_interseccion = 0;

    /* Interseccion con esfera */
    if (objeto->tipo == TIPO_ESFERA)
    {
        hay_interseccion = intersecar_esfera(
            escena->ojo,
            direccion,
            &objeto->u.esfera,
            &t
        );
    }

    /* Interseccion con poligono */
    else if (objeto->tipo == TIPO_POLIGONO)
    {
        hay_interseccion = intersecar_poligono(
            escena->ojo,
            direccion,
            &objeto->u.poligono,
            &t
        );
    }

    /* Conservar la superficie mas cercana */
    if (hay_interseccion && t < mejor_t)
    {
        mejor_t = t;
        objeto_golpeado = objeto;
    }
}

    /* Calcular la normal en el punto de interseccion */
if (objeto_golpeado != NULL)
{
    /* Punto donde el rayo intersecta la superficie mas cercana */
    Vec3 punto_interseccion = punto_en_rayo(
        escena->ojo,
        direccion,
        mejor_t
    );

    Vec3 normal = {0.0, 0.0, 0.0};
    Color color_base = {0.0, 0.0, 0.0};

    /*
     * Identificar el tipo de objeto para obtener
     * su normal y su color.
     */

    /* CASO 1: ESFERA */
    if (objeto_golpeado->tipo == TIPO_ESFERA)
    {
        const Esfera *esfera = &objeto_golpeado->u.esfera;

        /* Vector desde el centro hasta el punto de impacto */
        Vec3 vector_normal = vec_restar(
            punto_interseccion,
            esfera->centro
        );

        /* Normal unitaria de la esfera */
        normal = vec_normalizar(vector_normal);

        /* Color original de la esfera */
        color_base = esfera->color;
    }

    /* CASO 2: POLIGONO */
    else if (objeto_golpeado->tipo == TIPO_POLIGONO)
    {
        const Poligono *poligono = &objeto_golpeado->u.poligono;

        /* Normal calculada al cargar el poligono */
        normal = poligono->normal;

        /* Color original del poligono */
        color_base = poligono->color;

        /*
         * Orientar la normal hacia el lado desde
         * el cual llega el rayo de la camara.
         */
        if (vec_punto(normal, direccion) > 0.0)
        {
            normal = vec_escalar(normal, -1.0);
        }
    }

    /*
     * ILUMINACION
     *
     * Se conserva el modelo original:
     * ambiental + difusa de Lambert +
     * multiples luces + atenuacion.
     */

    /* Posiciones de las fuentes de luz */
    Vec3 luces[] = {
        {-3.0, 4.0, -2.0},  /* Luz principal */
        { 4.0, 2.0, -1.0}   /* Luz secundaria */
    };

    /* Intensidad de cada fuente */
    double intensidades[] = {
        1.0,  /* Luz principal */
        0.5   /* Luz secundaria */
    };

    /* Cantidad de fuentes de luz */
    int num_luces = sizeof(luces) / sizeof(luces[0]);

    /* Coeficiente de reflexion difusa */
    double Kd = 0.8;

    /* Iluminacion ambiental */
    double Ka = 0.3;
    double Ia = 0.5;

    /* Coeficientes de atenuacion */
    double C1 = 1.0;
    double C2 = 0.05;
    double C3 = 0.01;

    /* Inicializar con la contribucion ambiental */
    double intensidad_total = Ka * Ia;

    /* Recorrer todas las fuentes de luz */
    for (int k = 0; k < num_luces; k++)
    {
        /* Vector desde la interseccion hacia la luz */
        Vec3 vector_luz = vec_restar(
            luces[k],
            punto_interseccion
        );

        /* Distancia hacia la fuente */
        double distancia_luz = vec_longitud(vector_luz);

        /* Evitar division entre cero */
        if (distancia_luz <= 0.0)
        {
            continue;
        }

        /* Vector unitario hacia la luz */
        Vec3 L = vec_normalizar(vector_luz);

        /* Ley de Lambert */
        double producto_NL = vec_punto(normal, L);

        if (producto_NL < 0.0)
        {
            producto_NL = 0.0;
        }

        /* Atenuacion por distancia */
        double denominador = C1
                           + C2 * distancia_luz
                           + C3 * distancia_luz * distancia_luz;

        double Fatt = fmin(1.0, 1.0 / denominador);

        /* Contribucion difusa de la fuente actual */
        double intensidad_difusa =
            producto_NL * Kd * intensidades[k] * Fatt;

        /* Sumar la contribucion */
        intensidad_total += intensidad_difusa;
    }

    /* Limitar la intensidad al rango [0, 1] */
    intensidad_total = fmin(1.0, intensidad_total);

    /*
     * Aplicar la iluminacion al color original
     * del objeto, sea esfera o poligono.
     */
    color.r = color_base.r * intensidad_total;
    color.g = color_base.g * intensidad_total;
    color.b = color_base.b * intensidad_total;
}

return color;
}