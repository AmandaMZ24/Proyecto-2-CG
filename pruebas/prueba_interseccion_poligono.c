#include <stdio.h>
#include <math.h>

#include "poligonos.h"

static int comprobar(
    const char *nombre,
    Vec3 origen,
    Vec3 direccion,
    const Poligono *poligono,
    int esperado,
    double t_esperado
)
{
    double t = -1.0;

    int resultado = intersecar_poligono(
        origen,
        direccion,
        poligono,
        &t
    );

    int correcto = resultado == esperado;

    if (correcto && esperado) {
        correcto = fabs(t - t_esperado) < 1e-9;
    }

    printf("[%s] %s\n",
           correcto ? "OK" : "ERROR",
           nombre);

    return correcto;
}

int main(void)
{
    Vec3 cuadrado_vertices[] = {
        {0, 0, 2},
        {4, 0, 2},
        {4, 4, 2},
        {0, 4, 2}
    };

    Poligono cuadrado = {
        .vertices = cuadrado_vertices,
        .num_vertices = 4,
        .normal = {0, 0, 0},
        .color = {1, 0, 0}
    };

    Vec3 concavo_vertices[] = {
        {0, 0, 2},
        {4, 0, 2},
        {4, 1, 2},
        {1, 1, 2},
        {1, 4, 2},
        {0, 4, 2}
    };

    Poligono concavo = {
        .vertices = concavo_vertices,
        .num_vertices = 6,
        .normal = {0, 0, 0},
        .color = {0, 1, 0}
    };

    Vec3 direccion = {0, 0, 1};

    int aprobadas = 0;
    int total = 4;

    aprobadas += comprobar(
        "Cuadrado - interseccion central",
        (Vec3){2, 2, -5},
        direccion,
        &cuadrado,
        1,
        7.0
    );

    aprobadas += comprobar(
        "Cuadrado - rayo exterior",
        (Vec3){5, 2, -5},
        direccion,
        &cuadrado,
        0,
        0.0
    );

    aprobadas += comprobar(
        "Concavo - zona interior",
        (Vec3){0.5, 3, -5},
        direccion,
        &concavo,
        1,
        7.0
    );

    aprobadas += comprobar(
        "Concavo - hueco exterior",
        (Vec3){3, 3, -5},
        direccion,
        &concavo,
        0,
        0.0
    );

    printf("\nPruebas aprobadas: %d/%d\n",
           aprobadas, total);

    return aprobadas == total ? 0 : 1;
}
