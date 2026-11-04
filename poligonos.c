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
 * Calcula la interseccion entre un rayo y un poligono.
 *
 * Utiliza la normal previamente calculada
 * cuando el poligono fue cargado a la escena.
 *
 * Funciona para poligonos planos simples,
 * tanto convexos como concavos.
 */
int intersecar_poligono(
    Vec3 origen,
    Vec3 direccion,
    const Poligono *poligono,
    double *t
)
{
    const double EPSILON = 1e-9;

    if (poligono == NULL ||
        t == NULL ||
        poligono->vertices == NULL ||
        poligono->num_vertices < 3) {
        return 0;
    }

    /*
     * Obtener la normal almacenada.
     * Esta normal se calculo al cargar el poligono.
     */
    Vec3 normal = poligono->normal;

    /*
     * Comprobar que la normal sea valida.
     * Si es nula, intentamos calcularla.
     *
     * Este caso permite mantener compatibilidad
     * con pruebas que crean poligonos manualmente.
     */
    if (vec_longitud(normal) <= EPSILON) {
        if (!calcular_normal_poligono(poligono, &normal)) {
            return 0;
        }
    }

    /* Tomar el primer vertice como punto del plano */
    Vec3 punto_plano = poligono->vertices[0];

    double t_plano;

    /* Calcular interseccion entre rayo y plano */
    if (!intersecar_plano(
            origen,
            direccion,
            punto_plano,
            normal,
            &t_plano)) {
        return 0;
    }

    /* Calcular el punto tridimensional de impacto */
    Vec3 punto = vec_sumar(
        origen,
        vec_escalar(direccion, t_plano)
    );

    /*
     * Crear una copia local de la estructura.
     * No duplicamos los vertices, solamente
     * copiamos la estructura que los referencia.
     */
    Poligono poligono_local = *poligono;

    /*
     * Garantizar que la funcion punto_en_poligono
     * reciba una normal valida.
     */
    poligono_local.normal = normal;

    /* Verificar que el impacto este dentro */
    if (!punto_en_poligono(
            punto,
            &poligono_local)) {
        return 0;
    }

    /* Guardar el parametro de interseccion */
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

/*Comprueba que un poligono no tenga
 * aristas de longitud cero.
 *
 * Retorna 1 si todas las aristas son validas.
 * Retorna 0 si encuentra una arista degenerada.
 */
int validar_aristas(
    const Poligono *poligono
)
{
    const double EPSILON = 1e-9;

    if (poligono == NULL ||
        poligono->vertices == NULL ||
        poligono->num_vertices < 3) {
        return 0;
    }

    /* Verificar cada par de vertices consecutivos */
    for (int i = 0; i < poligono->num_vertices; i++)
    {
        int siguiente = (i + 1) % poligono->num_vertices;

        Vec3 diferencia = vec_restar(
            poligono->vertices[siguiente],
            poligono->vertices[i]
        );

        double longitud = vec_longitud(diferencia);

        if (!isfinite(longitud) || longitud <= EPSILON) {
            return 0;
        }
    }

    return 1;
}

/*
 * Calcula la orientacion de tres puntos 2D.
 *
 * Positivo: giro antihorario.
 * Negativo: giro horario.
 * Cero: puntos colineales.
 */
static double orientacion_2d(
    double ax, double ay,
    double bx, double by,
    double cx, double cy
)
{
    return (bx - ax) * (cy - ay)
         - (by - ay) * (cx - ax);
}

/*
 * Determina si dos segmentos 2D se intersectan.
 *
 * Incluye cruces, contactos en extremos
 * y segmentos colineales superpuestos.
 */
static int segmentos_intersectan(
    double ax, double ay,
    double bx, double by,
    double cx, double cy,
    double dx, double dy
)
{
    const double EPSILON = 1e-9;

    double o1 = orientacion_2d(
        ax, ay, bx, by, cx, cy
    );

    double o2 = orientacion_2d(
        ax, ay, bx, by, dx, dy
    );

    double o3 = orientacion_2d(
        cx, cy, dx, dy, ax, ay
    );

    double o4 = orientacion_2d(
        cx, cy, dx, dy, bx, by
    );

    /*
     * Caso general:
     * los extremos de cada segmento se encuentran
     * en lados opuestos del otro segmento.
     */
    int cruza_primero =
        (o1 > EPSILON && o2 < -EPSILON) ||
        (o1 < -EPSILON && o2 > EPSILON);

    int cruza_segundo =
        (o3 > EPSILON && o4 < -EPSILON) ||
        (o3 < -EPSILON && o4 > EPSILON);

    if (cruza_primero && cruza_segundo) {
        return 1;
    }

    /* Casos especiales: puntos sobre segmentos */
    if (fabs(o1) <= EPSILON &&
        punto_en_segmento(cx, cy, ax, ay, bx, by)) {
        return 1;
    }

    if (fabs(o2) <= EPSILON &&
        punto_en_segmento(dx, dy, ax, ay, bx, by)) {
        return 1;
    }

    if (fabs(o3) <= EPSILON &&
        punto_en_segmento(ax, ay, cx, cy, dx, dy)) {
        return 1;
    }

    if (fabs(o4) <= EPSILON &&
        punto_en_segmento(bx, by, cx, cy, dx, dy)) {
        return 1;
    }

    return 0;
}

int validar_poligono_simple(
    const Poligono *poligono
)
{
    if (poligono == NULL ||
        poligono->vertices == NULL ||
        poligono->num_vertices < 3) {
        return 0;
    }

    int n = poligono->num_vertices;

    Vec3 normal;

    if (!calcular_normal_poligono(
            poligono, &normal)) {
        return 0;
    }

    /* Seleccionar el eje dominante de la normal */
    double ax = fabs(normal.x);
    double ay = fabs(normal.y);
    double az = fabs(normal.z);

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

    /*
     * Comparar cada arista con las demas.
     */
    for (int i = 0; i < n; i++)
    {
        int siguiente_i = (i + 1) % n;

        double aix, aiy;
        double bix, biy;

        proyectar_punto(
            poligono->vertices[i],
            eje,
            &aix, &aiy
        );

        proyectar_punto(
            poligono->vertices[siguiente_i],
            eje,
            &bix, &biy
        );

        for (int j = i + 1; j < n; j++)
        {
            int siguiente_j = (j + 1) % n;

            /*
             * No comparar aristas adyacentes:
             * comparten un vertice por definicion.
             */
            if (j == i ||
                j == siguiente_i ||
                siguiente_j == i) {
                continue;
            }

            double cjx, cjy;
            double djx, djy;

            proyectar_punto(
                poligono->vertices[j],
                eje,
                &cjx, &cjy
            );

            proyectar_punto(
                poligono->vertices[siguiente_j],
                eje,
                &djx, &djy
            );

            if (segmentos_intersectan(
                    aix, aiy,
                    bix, biy,
                    cjx, cjy,
                    djx, djy)) {
                return 0;
            }
        }
    }

    return 1;
}