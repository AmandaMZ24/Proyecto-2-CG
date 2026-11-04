#include <stdio.h>

#include "poligonos.h"

int main(void)
{
    /* Cuadrado convexo */
    Vec3 vertices_cuadrado[] = {
        {0.0, 0.0, 2.0},
        {4.0, 0.0, 2.0},
        {4.0, 4.0, 2.0},
        {0.0, 4.0, 2.0}
    };

    Poligono cuadrado = {
        .vertices = vertices_cuadrado,
        .num_vertices = 4,
        .normal = {0.0, 0.0, 1.0},
        .color = {1.0, 0.0, 0.0}
    };

    /* Poligono concavo con forma de L */
    Vec3 vertices_concavo[] = {
        {0.0, 0.0, 2.0},
        {4.0, 0.0, 2.0},
        {4.0, 1.0, 2.0},
        {1.0, 1.0, 2.0},
        {1.0, 4.0, 2.0},
        {0.0, 4.0, 2.0}
    };

    Poligono concavo = {
        .vertices = vertices_concavo,
        .num_vertices = 6,
        .normal = {0.0, 0.0, 1.0},
        .color = {0.0, 1.0, 0.0}
    };

    struct {
        const char *nombre;
        Vec3 punto;
        const Poligono *poligono;
        int esperado;
    } pruebas[] = {
        {"Cuadrado - interior", {2, 2, 2}, &cuadrado, 1},
        {"Cuadrado - exterior", {5, 2, 2}, &cuadrado, 0},
        {"Cuadrado - borde", {0, 2, 2}, &cuadrado, 1},
        {"Concavo - interior", {0.5, 3, 2}, &concavo, 1},
        {"Concavo - exterior", {3, 3, 2}, &concavo, 0},
        {"Concavo - interior inferior", {3, 0.5, 2}, &concavo, 1}
    };

    int total = sizeof(pruebas) / sizeof(pruebas[0]);
    int aprobadas = 0;

    for (int i = 0; i < total; i++) {
        int resultado = punto_en_poligono(
            pruebas[i].punto,
            pruebas[i].poligono
        );

        if (resultado == pruebas[i].esperado) {
            printf("[OK] %s\n", pruebas[i].nombre);
            aprobadas++;
        } else {
            printf("[ERROR] %s - esperado=%d obtenido=%d\n",
                   pruebas[i].nombre,
                   pruebas[i].esperado,
                   resultado);
        }
    }

    printf("\nPruebas aprobadas: %d/%d\n",
           aprobadas, total);

    return aprobadas == total ? 0 : 1;
}
