#include <stdio.h>
#include <math.h>

#include "poligonos.h"

int main(void)
{
    /*
     * Cuadrilatero sobre el plano z = x + 2.
     * Sus vertices son coplanares.
     */
    Vec3 vertices[] = {
        {0, 0, 2},
        {2, 0, 4},
        {2, 2, 4},
        {0, 2, 2}
    };

    Poligono poligono = {
        .vertices = vertices,
        .num_vertices = 4
    };

    Vec3 normal;

    int normal_valida =
        calcular_normal_poligono(&poligono, &normal);

    if (!normal_valida) {
        printf("[ERROR] No se pudo calcular la normal\n");
        return 1;
    }

    poligono.normal = normal;

    printf("Normal: (%.4f, %.4f, %.4f)\n",
           normal.x, normal.y, normal.z);

    int coplanar = validar_coplanaridad(&poligono);

    printf("[%s] Coplanaridad\n",
           coplanar ? "OK" : "ERROR");

    Vec3 origen = {1, 1, -5};
    Vec3 direccion = {0, 0, 1};

    double t = -1;

    int intersecta = intersecar_poligono(
        origen,
        direccion,
        &poligono,
        &t
    );

    int t_correcto =
        intersecta && fabs(t - 8.0) < 1e-9;

    printf("[%s] Interseccion\n",
           t_correcto ? "OK" : "ERROR");

    if (intersecta) {
        printf("t = %.4f\n", t);
    }

    if (coplanar && t_correcto) {
        printf("\nPRUEBA APROBADA\n");
        return 0;
    }

    return 1;
}
