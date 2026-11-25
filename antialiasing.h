#ifndef ANTIALIASING_H
#define ANTIALIASING_H

#include "objetos.h"

double distancia_colores(
    Color a,
    Color b
);

Color antialiasing_pixel(
    const Escena *escena,
    int columna,
    int fila
);

#endif