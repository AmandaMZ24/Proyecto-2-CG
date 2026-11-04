#include <stdio.h>
#include "escena.h"

int main(void)
{
    Escena escena;

    if (!escena_leer(
            &escena,
            "escena_poligonos.txt")) {

        fprintf(
            stderr,
            "Error: %s\n",
            escena_error(&escena)
        );

        escena_liberar(&escena);
        return 1;
    }

    int esferas = 0;
    int poligonos = 0;

    for (int i = 0; i < escena.num_objetos; i++) {

        if (escena.objetos[i].tipo == TIPO_ESFERA) {
            esferas++;
        }

        else if (escena.objetos[i].tipo == TIPO_POLIGONO) {
            poligonos++;
        }
    }

    printf("Total de objetos: %d\n", escena.num_objetos);
    printf("Esferas: %d\n", esferas);
    printf("Poligonos: %d\n", poligonos);

    int correcto =
        esferas == 12 &&
        poligonos == 2 &&
        escena.num_objetos == 14;

    printf(
        "%s\n",
        correcto ? "PRUEBA APROBADA" : "PRUEBA FALLIDA"
    );

    escena_liberar(&escena);

    return correcto ? 0 : 1;
}

