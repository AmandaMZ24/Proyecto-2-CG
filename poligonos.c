#include "poligonos.h"
#include "vectores.h"

#include <math.h>
#include <stddef.h>

/*
 * Interseccion entre un rayo y un plano tridimensional.
 *
 * Ecuacion:
 *
 * t = N . (P0 - E) / (N . D)
 */
int intersecar_plano(
    Vec3 origen,
    Vec3 direccion,
    Vec3 punto_plano,
    Vec3 normal,
    double *t
)
{
    const double EPSILON = 1e-9;

    /* Verificar que el puntero sea valido */
    if (t == NULL) {
        return 0;
    }

    /*
     * Denominador de la ecuacion:
     *
     * N . D
     */
    double denominador = vec_punto(normal, direccion);

    /*
     * Si el denominador es cercano a cero,
     * el rayo es paralelo al plano.
     */
    if (fabs(denominador) < EPSILON) {
        return 0;
    }

    /*
     * Vector desde el origen del rayo
     * hasta un punto conocido del plano.
     */
    Vec3 diferencia = vec_restar(punto_plano, origen);

    /*
     * Numerador:
     *
     * N . (P0 - E)
     */
    double numerador = vec_punto(normal, diferencia);

    /* Calcular el parametro de interseccion */
    double t_calculado = numerador / denominador;

    /*
     * La interseccion debe encontrarse
     * delante del origen del rayo.
     */
    if (t_calculado <= EPSILON) {
        return 0;
    }

    *t = t_calculado;

    return 1;
}

/*
 * Proyecta un punto 3D a dos coordenadas.
 *
 * eje:
 * 0 -> conservar Y,Z
 * 1 -> conservar X,Z
 * 2 -> conservar X,Y
 */

static void proyectar_punto(
    Vec3 punto,
    int eje,
    double *u,
    double *v
)
{
    if (eje == 0) {
        *u = punto.y;
        *v = punto.z;
    }
    else if (eje == 1) {
        *u = punto.x;
        *v = punto.z;
    }
    else {
        *u = punto.x;
        *v = punto.y;
    }
}

/*
 * Determina si un punto 2D se encuentra
 * sobre el segmento A-B.
 */
static int punto_en_segmento(
    double px, double py,
    double ax, double ay,
    double bx, double by
)
{
    const double EPSILON = 1e-9;

    double dx = bx - ax;
    double dy = by - ay;

    double cruz =
        (px - ax) * dy -
        (py - ay) * dx;

    double escala = hypot(dx, dy);

    if (escala <= EPSILON) {
        return hypot(px - ax, py - ay) <= EPSILON;
    }

    /* Distancia perpendicular aproximada */
    if (fabs(cruz) / escala > EPSILON) {
        return 0;
    }

    double producto =
        (px - ax) * (px - bx) +
        (py - ay) * (py - by);

    return producto <= EPSILON;
}


int punto_en_poligono(
    Vec3 punto,
    const Poligono *poligono
)
{
    if (poligono == NULL ||
        poligono->vertices == NULL ||
        poligono->num_vertices < 3) {
        return 0;
    }

    Vec3 n = poligono->normal;

    double ax = fabs(n.x);
    double ay = fabs(n.y);
    double az = fabs(n.z);

    /* Seleccionar eje dominante de la normal */
    int eje;

    if (ax >= ay && ax >= az) {
        eje = 0;
    }
    else if (ay >= ax && ay >= az) {
        eje = 1;
    }
    else {
        eje = 2;
    }

    double px, py;

    proyectar_punto(punto, eje, &px, &py);

    int dentro = 0;

    for (int i = 0, j = poligono->num_vertices - 1;
         i < poligono->num_vertices;
         j = i++)
    {
        double xi, yi;
        double xj, yj;

        proyectar_punto(
            poligono->vertices[i],
            eje,
            &xi,
            &yi
        );

        proyectar_punto(
            poligono->vertices[j],
            eje,
            &xj,
            &yj
        );

        /* Primero revisar si cae sobre el borde */
        if (punto_en_segmento(
                px, py,
                xi, yi,
                xj, yj)) {
            return 1;
        }

        /*
         * Comprobar si el segmento cruza
         * el rayo horizontal desde el punto.
         */
        if ((yi > py) != (yj > py))
        {
            double x_interseccion =
                xi + (py - yi) *
                (xj - xi) / (yj - yi);

            if (px < x_interseccion) {
                dentro = !dentro;
            }
        }
    }

    return dentro;
}


int calcular_normal_poligono(
    const Poligono *poligono,
    Vec3 *normal
)
{
    const double EPSILON = 1e-9;

    if (poligono == NULL ||
        normal == NULL ||
        poligono->vertices == NULL ||
        poligono->num_vertices < 3) {
        return 0;
    }

    Vec3 p0 = poligono->vertices[0];

    for (int i = 1; i < poligono->num_vertices - 1; i++) {

        Vec3 a = vec_restar(
            poligono->vertices[i],
            p0
        );

        Vec3 b = vec_restar(
            poligono->vertices[i + 1],
            p0
        );

        Vec3 cruz = vec_cruz(a, b);

        double longitud = vec_longitud(cruz);

        if (longitud > EPSILON) {
            *normal = vec_escalar(cruz, 1.0 / longitud);
            return 1;
        }
    }

    return 0;
}


/*
 * Determina si un rayo intersecta un poligono.
 *
 * Retorna 1 si existe una interseccion valida.
 * Retorna 0 en caso contrario.
 *
 * Guarda en t la distancia parametrica.
 */
int intersecar_poligono(
    Vec3 origen,
    Vec3 direccion,
    const Poligono *poligono,
    double *t
)
{
    if (poligono == NULL || 
        t == NULL ||
        poligono->vertices == NULL ||
        poligono->num_vertices < 3) {
        return 0;
    }

    /* Calcular la normal del poligono */
    Vec3 normal;

    if (!calcular_normal_poligono(poligono, &normal)) {
        return 0;
    }

    /* Obtener un punto del plano */
    Vec3 punto_plano = poligono->vertices[0];

    double t_plano;

    /*Comprobar intersección con el plano */
    if (!intersecar_plano(
            origen,
            direccion,
            punto_plano,
            normal,
            &t_plano)) {
        return 0;
    }

    /* Calcular el punto de intersección */
    Vec3 punto = vec_sumar(
        origen,
        vec_escalar(direccion, t_plano)
    );

    /*Usamos una copia local porque punto_en_poligono utiliza el campo normal del poligono */
    Poligono poligono_local = *poligono;
    poligono_local.normal = normal;

    /* Comprobar si el punto de intersección está dentro del polígono */
    if (!punto_en_poligono(punto, &poligono_local)) {
        return 0;
    }

    *t = t_plano;

    return 1;

}

/*
 * Verifica que todos los vertices de un poligono
 * pertenezcan al mismo plano.
 *
 * Retorna:
 * 1 si los vertices son coplanares.
 * 0 si no lo son o el poligono es invalido.
 */
int validar_coplanaridad(
    const Poligono *poligono
)
{
    const double EPSILON = 1e-9;

    if (poligono == NULL || 
        poligono->vertices == NULL || 
        poligono->num_vertices < 3) {
        return 0;
    }

    Vec3 normal;

    /* Obtener una normal valida */
    if (!calcular_normal_poligono(poligono, &normal)) {
        return 0;
    }

    Vec3 p0 = poligono->vertices[0];

    /* Verificar cada vertice */
    for (int i = 1; i < poligono->num_vertices; i++) {

        Vec3 diferencia = vec_restar(
            poligono->vertices[i],
            p0
        );

        double distancia = fabs(
            vec_punto(normal, diferencia)
        );

        if (distancia > EPSILON) {
            return 0;
        }
    }

    return 1;
}
