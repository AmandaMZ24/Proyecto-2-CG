#include <stdio.h>

#include "poligonos.h"

int main(void)
{
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

    Vec3 vertices_invalidos[] = {
        {0, 0, 2},
        {4, 0, 2},
        {4, 4, 3},
        {0, 4, 2}
    };

    Poligono invalido = {
        .vertices = vertices_invalidos,
        .num_vertices = 4
    };

    int resultado1 = validar_coplanaridad(&valido);
    int resultado2 = validar_coplanaridad(&invalido);

    printf("[%s] Poligono coplanar\n",
           resultado1 == 1 ? "OK" : "ERROR");

    printf("[%s] Poligono no coplanar\n",
           resultado2 == 0 ? "OK" : "ERROR");

    if (resultado1 == 1 && resultado2 == 0) {
        printf("\nPruebas aprobadas: 2/2\n");
        return 0;
    }

    return 1;
}
