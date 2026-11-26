#ifndef TEXTURAS_H
#define TEXTURAS_H

#include "objetos.h"

/* Cargar una imagen AVS */
int cargar_textura_avs(
    const char *ruta,
    Textura *textura
);

/* Liberar la memoria de una textura */
void liberar_textura(
    Textura *textura
);

/* Obtener el color asociado a coordenadas UV */
Color color_textura(
    const Textura *textura,
    double u,
    double v
);

/* Coordenadas UV de un poligono */
int uv_poligono(
    Vec3 punto,
    const Poligono *poligono,
    double *u,
    double *v
);

/* Coordenadas UV de un cilindro */
int uv_cilindro(
    Vec3 punto,
    const Cilindro *cilindro,
    Vec3 G,
    double *u,
    double *v
);

/* Coordenadas UV de una esfera */
int uv_esfera(
    Vec3 punto,
    const Esfera *esfera,
    Vec3 norte,
    Vec3 G,
    double *u,
    double *v
);

/* Obtener el color base o texturizado del objeto */
Color color_superficie(
    const Escena *escena,
    const Objeto *objeto,
    Vec3 punto,
    Color color_base
);

#endif