#include <stdio.h>
#include "poligonos.h"

int main(void)
{
    /* Cuadrado valido */
    Vec3 vertices_cuadrado[] = {
        {0, 0, 2},
        {4, 0, 2},
        {4, 4, 2},
        {0, 4, 2}
    };

    Poligono cuadrado = {
        .vertices = vertices_cuadrado,
        .num_vertices = 4
    };

    /* Poligono concavo valido */
    Vec3 vertices_concavo[] = {
        {0, 0, 2},
        {4, 0, 2},
        {4, 1, 2},
        {1, 1, 2},
        {1, 4, 2},
        {0, 4, 2}
    };

    Poligono concavo = {
        .vertices = vertices_concavo,
        .num_vertices = 6
    };

    /* Poligono autointersecado */
    Vec3 vertices_mariposa[] = {
        {0, 0, 2},
        {4, 4, 2},
        {0, 4, 2},
        {4, 0, 2}
    };

    Poligono mariposa = {
        .vertices = vertices_mariposa,
        .num_vertices = 4
    };

    int r1 = validar_poligono_simple(&cuadrado);
    int r2 = validar_poligono_simple(&concavo);
    int r3 = validar_poligono_simple(&mariposa);

    printf("[%s] Cuadrado valido\n",
           r1 == 1 ? "OK" : "ERROR");

    printf("[%s] Concavo valido\n",
           r2 == 1 ? "OK" : "ERROR");

    printf("[%s] Mariposa rechazada\n",
           r3 == 0 ? "OK" : "ERROR");

    int aprobadas =
        (r1 == 1) + (r2 == 1) + (r3 == 0);

    printf("\nPruebas aprobadas: %d/3\n", aprobadas);

    return aprobadas == 3 ? 0 : 1;
}
