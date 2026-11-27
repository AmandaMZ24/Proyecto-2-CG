#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

#include "texturas.h"
#include "vectores.h"

#define EPSILON 1e-9
#define PI 3.14159265358979323846

/* Limitar un valor a un intervalo */
static double limitar(
    double valor,
    double minimo,
    double maximo
)
{
    return fmax(minimo, fmin(maximo, valor));
}

/* Leer entero de 32 bits en big-endian */
static uint32_t leer_big_endian(
    const unsigned char *bytes
)
{
    return ((uint32_t)bytes[0] << 24)
         | ((uint32_t)bytes[1] << 16)
         | ((uint32_t)bytes[2] << 8)
         |  (uint32_t)bytes[3];
}

int cargar_textura_avs(
    const char *ruta,
    Textura *textura
)
{
    if (ruta == NULL || textura == NULL) {
        return 0;
    }

    FILE *archivo = fopen(ruta, "rb");

    if (archivo == NULL) {
        return 0;
    }

    unsigned char cabecera[8];

    /* Leer ancho y alto */
    if (fread(cabecera, 1, 8, archivo) != 8) {
        fclose(archivo);
        return 0;
    }

    uint32_t ancho = leer_big_endian(
        &cabecera[0]
    );

    uint32_t alto = leer_big_endian(
        &cabecera[4]
    );

    /* Validar dimensiones */
    if (ancho == 0 || alto == 0 ||
        ancho > INT_MAX || alto > INT_MAX ||
        (size_t)ancho > SIZE_MAX / 4 / alto) {

        fclose(archivo);
        return 0;
    }

    size_t cantidad =
        (size_t)ancho * alto * 4;

    /* Reservar memoria para los pixeles */
    unsigned char *pixeles = malloc(cantidad);

    if (pixeles == NULL) {
        fclose(archivo);
        return 0;
    }

    /* Leer todos los pixeles ARGB */
    if (fread(
            pixeles, 1, cantidad, archivo
        ) != cantidad) {

        free(pixeles);
        fclose(archivo);
        return 0;
    }

    fclose(archivo);

    textura->ancho = (int)ancho;
    textura->alto = (int)alto;
    textura->pixeles = pixeles;

    return 1;
}

void liberar_textura(Textura *textura)
{
    if (textura == NULL) {
        return;
    }

    free(textura->pixeles);

    textura->pixeles = NULL;
    textura->ancho = 0;
    textura->alto = 0;
}

Color color_textura(
    const Textura *textura,
    double u,
    double v
)
{
    Color negro = {0.0, 0.0, 0.0};

    if (textura == NULL ||
        textura->pixeles == NULL ||
        textura->ancho <= 0 ||
        textura->alto <= 0 ||
        !isfinite(u) ||
        !isfinite(v)) {

        return negro;
    }

    /* Limitar coordenadas al intervalo valido */
    u = limitar(u, 0.0, 1.0);
    v = limitar(v, 0.0, 1.0);

    /* Convertir coordenadas UV a pixeles */
    int x = (int)(
        u * (textura->ancho - 1) + 0.5
    );

    int y = (int)(
        v * (textura->alto - 1) + 0.5
    );

    size_t indice =
        ((size_t)y * textura->ancho + x) * 4;

    /* Ignorar alfa y recuperar RGB */
    Color color = {
        textura->pixeles[indice + 1] / 255.0,
        textura->pixeles[indice + 2] / 255.0,
        textura->pixeles[indice + 3] / 255.0
    };

    return color;
}

static int uv_triangulo(
    Vec3 punto,
    const Objeto *objeto,
    double *u,
    double *v
)
{
    if (objeto == NULL ||
        objeto->tipo != TIPO_POLIGONO ||
        objeto->u.poligono.num_vertices != 3 ||
        !objeto->tiene_uv ||
        u == NULL ||
        v == NULL) {

        return 0;
    }

    const Poligono *p =
        &objeto->u.poligono;

    Vec3 A = p->vertices[0];
    Vec3 B = p->vertices[1];
    Vec3 C = p->vertices[2];

    /* Vectores del triangulo */
    Vec3 AB = vec_restar(B, A);
    Vec3 AC = vec_restar(C, A);
    Vec3 AP = vec_restar(punto, A);

    /* Productos punto */
    double d00 = vec_punto(AB, AB);
    double d01 = vec_punto(AB, AC);
    double d11 = vec_punto(AC, AC);
    double d20 = vec_punto(AP, AB);
    double d21 = vec_punto(AP, AC);

    double denominador =
        d00 * d11 - d01 * d01;

    /* Evitar triangulos degenerados */
    if (d00 <= 0.0 ||
        d11 <= 0.0 ||
        denominador <= 1e-12 * d00 * d11) {

        return 0;
    }

    /* Coordenadas baricentricas */
    double beta =
        (d11 * d20 - d01 * d21) /
        denominador;

    double gamma =
        (d00 * d21 - d01 * d20) /
        denominador;

    double alpha =
        1.0 - beta - gamma;

    /* Interpolar coordenadas U */
    *u =
        alpha * objeto->uv[0][0] +
        beta  * objeto->uv[1][0] +
        gamma * objeto->uv[2][0];

    /* Interpolar coordenadas V */
    *v =
        alpha * objeto->uv[0][1] +
        beta  * objeto->uv[1][1] +
        gamma * objeto->uv[2][1];

    return isfinite(*u) && isfinite(*v);
}

int uv_poligono(
    Vec3 punto,
    const Poligono *poligono,
    double *u,
    double *v
)
{
    if (poligono == NULL ||
        poligono->vertices == NULL ||
        poligono->num_vertices < 3 ||
        u == NULL ||
        v == NULL) {

        return 0;
    }

    Vec3 O = poligono->vertices[0];

    /* Primer vector director */
    Vec3 U = vec_restar(
        poligono->vertices[1],
        O
    );

    if (vec_longitud(U) <= EPSILON ||
        vec_longitud(poligono->normal) <= EPSILON) {
        return 0;
    }

    U = vec_normalizar(U);

    Vec3 N = vec_normalizar(
        poligono->normal
    );

    /* Segundo vector perpendicular */
    Vec3 V = vec_normalizar(
        vec_cruz(N, U)
    );

    if (vec_longitud(V) <= EPSILON) {
        return 0;
    }

    double min_u = 0.0;
    double max_u = 0.0;
    double min_v = 0.0;
    double max_v = 0.0;

    /* Encontrar el rectangulo envolvente */
    for (int i = 0;
         i < poligono->num_vertices;
         i++) {

        Vec3 W = vec_restar(
            poligono->vertices[i],
            O
        );

        double s = vec_punto(W, U);
        double t = vec_punto(W, V);

        min_u = fmin(min_u, s);
        max_u = fmax(max_u, s);

        min_v = fmin(min_v, t);
        max_v = fmax(max_v, t);
    }

    double ancho = max_u - min_u;
    double alto = max_v - min_v;

    if (ancho <= EPSILON ||
        alto <= EPSILON) {

        return 0;
    }

    /* Proyectar punto de impacto */
    Vec3 W = vec_restar(punto, O);

    double s = vec_punto(W, U);
    double t = vec_punto(W, V);

    /* Normalizar coordenadas */
    *u = (s - min_u) / ancho;
    *v = 1.0 - (t - min_v) / alto;

    return 1;
}

static Vec3 referencia_perpendicular(
    Vec3 eje,
    Vec3 G
)
{
    /* Eliminar componente paralela al eje */
    G = vec_restar(
        G,
        vec_escalar(
            eje,
            vec_punto(G, eje)
        )
    );

    /* Elegir referencia alternativa */
    if (vec_longitud(G) <= EPSILON) {

        Vec3 auxiliar =
            fabs(eje.x) < 0.9
            ? (Vec3){1.0, 0.0, 0.0}
            : (Vec3){0.0, 1.0, 0.0};

        G = vec_restar(
            auxiliar,
            vec_escalar(
                eje,
                vec_punto(auxiliar, eje)
            )
        );
    }

    return vec_normalizar(G);
}

int uv_cilindro(
    Vec3 punto,
    const Cilindro *cilindro,
    Vec3 G,
    double *u,
    double *v
)
{
    if (cilindro == NULL ||
        u == NULL ||
        v == NULL) {

        return 0;
    }

    Vec3 Q = vec_normalizar(
        cilindro->eje
    );

    double altura =
        cilindro->d_max - cilindro->d_min;

    if (altura <= EPSILON ||
        vec_longitud(Q) <= EPSILON) {
        return 0;
    }

    Vec3 W = vec_restar(
        punto,
        cilindro->ancla
    );

    /* Distancia longitudinal */
    double d = vec_punto(W, Q);

    /* Coordenada vertical */
    *v = 1.0 -
        (d - cilindro->d_min) / altura;

    /* Normal radial geometrica */
    Vec3 N = vec_restar(
        W,
        vec_escalar(Q, d)
    );

    if (vec_longitud(N) <= EPSILON) {
        return 0;
    }

    N = vec_normalizar(N);

    /* Referencia angular */
    G = referencia_perpendicular(Q, G);

    Vec3 H = vec_cruz(Q, G);

    /* Angulo alrededor del cilindro */
    double angulo = atan2(
        vec_punto(N, H),
        vec_punto(N, G)
    );

    if (angulo < 0.0) {
        angulo += 2.0 * PI;
    }

    *u = angulo / (2.0 * PI);

    return 1;
}

int uv_esfera(
    Vec3 punto,
    const Esfera *esfera,
    Vec3 norte,
    Vec3 G,
    double *u,
    double *v
)
{
    if (esfera == NULL ||
        u == NULL ||
        v == NULL) {

        return 0;
    }

    Vec3 N = vec_restar(
        punto,
        esfera->centro
    );

    if (vec_longitud(N) <= EPSILON) {
        return 0;
    }

    N = vec_normalizar(N);

    /* Eje norte por defecto */
    if (vec_longitud(norte) <= EPSILON) {
        norte = (Vec3){0.0, 1.0, 0.0};
    }

    norte = vec_normalizar(norte);

    /* Coordenada vertical */
    double coseno = limitar(
        vec_punto(norte, N),
        -1.0,
        1.0
    );

    *v = acos(coseno) / PI;

    /* Referencia para el meridiano inicial */
    G = referencia_perpendicular(norte, G);

    Vec3 H = vec_cruz(norte, G);

    double x = vec_punto(N, G);
    double y = vec_punto(N, H);

    /* Caso especial: polos */
    if (fabs(x) <= EPSILON &&
        fabs(y) <= EPSILON) {

        *u = 0.0;
        return 1;
    }

    double angulo = atan2(y, x);

    if (angulo < 0.0) {
        angulo += 2.0 * PI;
    }

    *u = angulo / (2.0 * PI);

    return 1;
}

Color color_superficie(
    const Escena *escena,
    const Objeto *objeto,
    Vec3 punto,
    Color color_base
)
{
    if (escena == NULL ||
        objeto == NULL ||
        escena->texturas == NULL ||
        objeto->textura <= 0 ||
        objeto->textura > escena->num_texturas) {

        return color_base;
    }

    const Textura *textura =
        &escena->texturas[objeto->textura - 1];

    double u = 0.0;
    double v = 0.0;

    int valido = 0;

    switch (objeto->tipo) {

        case TIPO_POLIGONO:
        
            if (objeto->tiene_uv) {

        /* Mapeo UV explicito */
        valido = uv_triangulo(
            punto,
            objeto,
            &u,
            &v
        );

    } else {

        /* Mapeo automatico original */
        valido = uv_poligono(
            punto,
            &objeto->u.poligono,
            &u,
            &v
        );
    }

    break;

        case TIPO_CILINDRO:

            valido = uv_cilindro(
                punto,
                &objeto->u.cilindro,
                objeto->textura_g,
                &u,
                &v
            );

            break;

        case TIPO_ESFERA:

            valido = uv_esfera(
                punto,
                &objeto->u.esfera,
                objeto->textura_norte,
                objeto->textura_g,
                &u,
                &v
            );

            break;

        default:
            return color_base;
    }

    if (!valido) {
        return color_base;
    }

    return color_textura(
        textura,
        u,
        v
    );
}