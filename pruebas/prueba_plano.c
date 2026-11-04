#include <stdio.h>
#include <math.h>

#include "poligonos.h"

int main(void)
{
    Vec3 origen = {0.0, 0.0, -5.0};

    Vec3 direccion = {0.0, 0.0, 1.0};

    Vec3 punto_plano = {0.0, 0.0, 2.0};

    Vec3 normal = {0.0, 0.0, 1.0};

    double t = 0.0;

    int resultado = intersecar_plano(
        origen,
        direccion,
        punto_plano,
        normal,
        &t
    );

    if (resultado) {
        printf("Interseccion detectada.\n");
        printf("Valor de t: %.2f\n", t);

        if (fabs(t - 7.0) < 1e-9) {
            printf("PRUEBA APROBADA\n");
        } else {
            printf("PRUEBA FALLIDA\n");
            return 1;
        }
    } else {
        printf("No se detecto interseccion.\n");
        return 1;
    }

    return 0;
}
