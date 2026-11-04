#include <stdio.h>
#include "poligonos.h"

int main(void)
{
    /* Cuadrado valido */
    Vec3 vertices_validos[] = {
        {0, 0, 2},
        {4, 0, 2},
        {4, 4, 2},
        {0, 4, 2}
    };

    Poligono valido = {
        .vertices = vertices_validos,
        .num_vertices = 4
    };

    /* Poligono con vertices repetidos */
    Vec3 vertices_invalidos[] = {
        {0, 0, 2},
        {4, 0, 2},
        {4, 0, 2},
        {4, 4, 2},
        {0, 4, 2}
    };

    Poligono invalido = {
        .vertices = vertices_invalidos,
        .num_vertices = 5
    };

    int resultado1 = validar_aristas(&valido);
    int resultado2 = validar_aristas(&invalido);

    printf("[%s] Cuadrado valido\n",
           resultado1 == 1 ? "OK" : "ERROR");

    printf("[%s] Arista degenerada\n",
           resultado2 == 0 ? "OK" : "ERROR");

    if (resultado1 == 1 && resultado2 == 0) {
        printf("\nPruebas aprobadas: 2/2\n");
        return 0;
    }

    return 1;
}
