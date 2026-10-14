#include <float.h>
#include <math.h>
#include "raytracing.h"
#include "vectores.h"

int intersecar_esfera(Vec3 origen, Vec3 direccion, const Esfera *esfera, double *t)
{
    if (!esfera || !t || esfera->radio <= 0.0) return 0;
    Vec3 oc = vec_restar(origen, esfera->centro);
    double a = vec_punto(direccion, direccion);
    if (a <= 0.0) return 0;
    double b = 2.0 * vec_punto(oc, direccion);
    double c = vec_punto(oc, oc) - esfera->radio * esfera->radio;
    double discriminante = b*b - 4.0*a*c;
    if (discriminante < 0.0) return 0;
    double raiz = sqrt(discriminante);
    double t1 = (-b - raiz) / (2.0*a);
    double t2 = (-b + raiz) / (2.0*a);
    const double epsilon = 1e-9;
    if (t1 > epsilon) { *t = t1; return 1; }
    if (t2 > epsilon) { *t = t2; return 1; }
    return 0;
}

Color trazar_pixel(const Escena *escena, int columna, int fila)
{
    Color negro = {0.0, 0.0, 0.0};
    if (!escena || escena->ancho <= 0 || escena->alto <= 0 ||
        columna < 0 || columna >= escena->ancho || fila < 0 || fila >= escena->alto)
        return negro;

    /* El centro del pixel se proyecta en el plano de imagen z=0. */
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
    for (int i = 0; i < escena->num_objetos; ++i) {
        const Objeto *objeto = &escena->objetos[i];
        if (objeto->tipo != TIPO_ESFERA) continue;
        double t;
        if (intersecar_esfera(escena->ojo, direccion, &objeto->u.esfera, &t) &&
            t < mejor_t) {
            mejor_t = t;
            color = objeto->u.esfera.color;
        }
    }
    return color;
}
