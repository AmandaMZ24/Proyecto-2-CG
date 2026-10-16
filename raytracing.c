#include <float.h>
#include <math.h>
#include <stddef.h>
#include "raytracing.h"
#include "vectores.h"

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

    double mejor_t = DBL_MAX;
    Color color = escena->fondo;
    const Esfera *esfera_golpeada = NULL;

    for (int i = 0; i < escena->num_objetos; ++i) {
        const Objeto *objeto = &escena->objetos[i];

        if (objeto->tipo != TIPO_ESFERA) {
            continue;
        }

        double t;

        if (intersecar_esfera(escena->ojo, direccion, &objeto->u.esfera, &t) &&
            t < mejor_t) {
            mejor_t = t;
            esfera_golpeada = &objeto->u.esfera;
            color = objeto->u.esfera.color;
        }
    }

    /* Calcular la normal en el punto de interseccion */
if (esfera_golpeada != NULL)
{
    /* Punto donde el rayo golpea la esfera */
    Vec3 punto_interseccion = punto_en_rayo(
        escena->ojo,
        direccion,
        mejor_t
    );

    /* Vector desde el centro hasta el punto de impacto */
    Vec3 vector_normal = vec_restar(
        punto_interseccion,
        esfera_golpeada->centro
    );

   /* Convertir la normal en un vector unitario */
    Vec3 normal = vec_normalizar(vector_normal);



    /* Posiciones de las fuentes de luz */
    Vec3 luces[] = {
        {-3.0, 4.0, -2.0},  /* Luz principal */
        { 4.0, 2.0, -1.0}   /* Luz secundaria */
    };

    /* Intensidad de cada fuente */
    double intensidades[] = {
        1.0,   /* Luz principal */
        0.5    /* Luz secundaria */
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

    /* Aplicar la iluminacion al color original */
    color.r = esfera_golpeada->color.r * intensidad_total;
    color.g = esfera_golpeada->color.g * intensidad_total;
    color.b = esfera_golpeada->color.b * intensidad_total;
}

    return color;
}