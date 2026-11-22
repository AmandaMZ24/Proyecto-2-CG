/*
 * prueba_opticos.c
 *
 * Prueba de los efectos opticos del trazador:
 *   - lectura de "espejo" y "transparencia" en el archivo de escena
 *   - sombras atenuadas por la transparencia del obstaculo
 *   - transparencia NO refractiva: mezcla con tinte del color base
 *   - espejo: rayo reflejado con la direccion correcta
 *   - combinacion espejo + transparencia
 *   - limite de recursion frente a espejos enfrentados
 *
 * Compilar y ejecutar desde la raiz del proyecto con:  make prueba
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../escena.h"
#include "../raytracing.h"
#include "../vectores.h"

static int fallos = 0;

static void ok(int cond, const char *msg)
{
    printf("%s %s\n", cond ? "[ok]  " : "[FALLO]", msg);
    if (!cond) fallos++;
}

static int casi(double a, double b)
{
    return fabs(a - b) < 1e-9;
}

static int color_casi(Color a, Color b)
{
    return casi(a.r, b.r) && casi(a.g, b.g) && casi(a.b, b.b);
}

/* ================================================================== */
/*  1. Lectura de espejo y transparencia desde el archivo              */
/* ================================================================== */

/*
 * Escribe una escena valida con 8 esferas; la primera recibe la
 * linea `extra` (opcional) justo despues de definirla.
 */
static void escribir_escena(const char *ruta, const char *extra)
{
    FILE *f = fopen(ruta, "w");
    if (f == NULL) return;

    fprintf(f, "resolucion 10 10\n");
    fprintf(f, "ojo 0 0 -5\n");
    fprintf(f, "ventana -1 -1 1 1\n");
    fprintf(f, "fondo 0 0 0\n");
    fprintf(f, "ambiente 0.5\n");
    fprintf(f, "esfera 0 0 2 1 1 0 0\n");
    if (extra != NULL) {
        fprintf(f, "%s\n", extra);
    }
    for (int i = 1; i < 8; i++) {
        fprintf(f, "esfera %d 0 2 1 1 0 0\n", i);
    }
    fprintf(f, "fin\n");
    fclose(f);
}

static void prueba_lectura(void)
{
    Escena e;

    printf("Lectura de propiedades opticas:\n");

    /* espejo y transparencia por separado */
    escribir_escena("/tmp/prueba_espejo.txt", "espejo 0.65");
    ok(escena_leer(&e, "/tmp/prueba_espejo.txt"),
       "\"espejo 0.65\" se lee sin error");
    ok(casi(e.objetos[0].u.esfera.material.reflejo, 0.65),
       "reflejo = 0.65 en la primera esfera");
    ok(casi(e.objetos[1].u.esfera.material.reflejo, 0.0),
       "el reflejo no se propaga a las demas esferas");
    escena_liberar(&e);

    escribir_escena("/tmp/prueba_transp.txt", "transparencia 0.8");
    ok(escena_leer(&e, "/tmp/prueba_transp.txt"),
       "\"transparencia 0.8\" se lee sin error");
    ok(casi(e.objetos[0].u.esfera.material.transparencia, 0.8),
       "transparencia = 0.8 en la primera esfera");
    escena_liberar(&e);

    /* material con seis valores fija ambas propiedades */
    escribir_escena("/tmp/prueba_mat6.txt", "material 0.2 0.7 0.3 64 0.4 0.9");
    ok(escena_leer(&e, "/tmp/prueba_mat6.txt"),
       "\"material\" con 6 valores se lee sin error");
    ok(casi(e.objetos[0].u.esfera.material.ka, 0.2) &&
       casi(e.objetos[0].u.esfera.material.reflejo, 0.4) &&
       casi(e.objetos[0].u.esfera.material.transparencia, 0.9),
       "material de 6 valores: ka, reflejo y transparencia guardados");
    escena_liberar(&e);

    /* material de 4 valores conserva las propiedades opticas */
    escribir_escena("/tmp/prueba_mat4.txt",
                    "espejo 0.6\nmaterial 0.1 0.5 0.2 16");
    ok(escena_leer(&e, "/tmp/prueba_mat4.txt"),
       "espejo seguido de material de 4 valores se lee sin error");
    ok(casi(e.objetos[0].u.esfera.material.ka, 0.1) &&
       casi(e.objetos[0].u.esfera.material.reflejo, 0.6),
       "\"material\" de 4 valores no borra el reflejo previo");
    escena_liberar(&e);

    /* Validaciones */
    escribir_escena("/tmp/prueba_e1.txt", "espejo 1.5");
    ok(!escena_leer(&e, "/tmp/prueba_e1.txt"), "espejo fuera de [0, 1] rechazado");
    escena_liberar(&e);

    escribir_escena("/tmp/prueba_e2.txt", "transparencia -0.2");
    ok(!escena_leer(&e, "/tmp/prueba_e2.txt"),
       "transparencia negativa rechazada");
    escena_liberar(&e);

    escribir_escena("/tmp/prueba_e3.txt", "espejo");
    ok(!escena_leer(&e, "/tmp/prueba_e3.txt"), "\"espejo\" sin valor rechazado");
    escena_liberar(&e);

    /* Sin objeto previo */
    escribir_escena("/tmp/prueba_e4.txt", NULL);
    {
        FILE *f = fopen("/tmp/prueba_e4.txt", "a");
        if (f != NULL) {
            /* reabrir para insertar el espejo antes de las esferas */
            fclose(f);
        }
    }
    {
        FILE *f = fopen("/tmp/prueba_e4.txt", "w");
        if (f != NULL) {
            fprintf(f, "resolucion 10 10\nojo 0 0 -5\n");
            fprintf(f, "ventana -1 -1 1 1\nfondo 0 0 0\n");
            fprintf(f, "espejo 0.5\n");
            for (int i = 0; i < 8; i++) {
                fprintf(f, "esfera %d 0 2 1 1 0 0\n", i);
            }
            fprintf(f, "fin\n");
            fclose(f);
        }
    }
    ok(!escena_leer(&e, "/tmp/prueba_e4.txt"),
       "\"espejo\" sin objeto previo rechazado");
    escena_liberar(&e);

    remove("/tmp/prueba_espejo.txt");
    remove("/tmp/prueba_transp.txt");
    remove("/tmp/prueba_mat6.txt");
    remove("/tmp/prueba_mat4.txt");
    remove("/tmp/prueba_e1.txt");
    remove("/tmp/prueba_e2.txt");
    remove("/tmp/prueba_e3.txt");
    remove("/tmp/prueba_e4.txt");
}

/* ================================================================== */
/*  Escena base: suelo horizontal y una luz sobre el                   */
/* ================================================================== */

/*
 * Construye un escenario con suelo en y = 0, una luz en (0, 6, 0) y
 * una esfera en (0, 3, 0) que puede bloquear la luz hacia el origen.
 * Devuelve 1 si todo se agrego correctamente.
 */
static int escena_sombra(Escena *e, double transparencia_esfera)
{
    Vec3 suelo[4] = {
        {10.0, 0.0, -10.0}, {-10.0, 0.0, -10.0},
        {-10.0, 0.0, 10.0}, {10.0, 0.0, 10.0}
    };
    /* ks = 0 en el suelo: el test verifica solo ambiente + difusa. */
    Material m_suelo   = {0.4, 0.8, 0.0, 48.0, 0.0, 0.0};
    Material m_esfera  = {0.3, 0.8, 0.6, 48.0, 0.0, 0.0};

    escena_inicializar(e);
    e->ojo = (Vec3){5.0, 10.0, -5.0};
    e->ancho = 4;
    e->alto = 4;
    e->ventana_xmin = -1; e->ventana_xmax = 1;
    e->ventana_ymin = -1; e->ventana_ymax = 1;
    e->fondo = (Color){0.0, 0.0, 0.0};
    e->ambiente = 0.2;

    if (!escena_agregar_poligono(e, suelo, 4, (Color){1.0, 1.0, 1.0},
                                 m_suelo)) {
        return 0;
    }

    m_esfera.transparencia = transparencia_esfera;
    if (!escena_agregar_esfera(e, (Vec3){0.0, 3.0, 0.0}, 1.0,
                               (Color){1.0, 1.0, 1.0}, m_esfera)) {
        return 0;
    }

    return escena_agregar_luz(e, (Vec3){0.0, 6.0, 0.0}, 1.0);
}

/* ================================================================== */
/*  2. Sombra atenuada por la transparencia del obstaculo              */
/* ================================================================== */

static void prueba_sombra_transparente(void)
{
    Escena e;
    Vec3   ojo = {5.0, 10.0, -5.0};
    Vec3   dir_a = {-5.0, -10.0, 5.0};   /* golpea el origen (0,0,0) */

    printf("Sombras y transparencia:\n");

    /* Fatt de la luz a distancia 6 con los coeficientes del trazador */
    double Fatt = 1.0 / (1.0 + 0.02 * 6.0 + 0.004 * 36.0);
    double difusa_total = 1.0 * 0.8 * 1.0 * Fatt * 1.0;   /* kd*Ip*Fatt*NdotL */
    double ambiente = 1.0 * 0.4 * 0.2;                    /* base*ka*Ia */

    /* Obstaculo opaco: sombra total, solo ambiente */
    ok(escena_sombra(&e, 0.0), "escena con obstaculo opaco");
    Color opaco = De_Que_Color(&e, ojo, dir_a);
    ok(casi(opaco.r, ambiente),
       "obstaculo opaco (transp = 0): el punto queda con solo ambiente");
    escena_liberar(&e);

    /* Obstaculo semitransparente: la luz se reduce en la misma medida */
    ok(escena_sombra(&e, 0.5), "escena con obstaculo al 50%");
    Color mitad = De_Que_Color(&e, ojo, dir_a);
    ok(casi(mitad.r, ambiente + 0.5 * difusa_total),
       "obstaculo al 50%: difusa reducida a la mitad");

    Material segundo_cristal = {0.3, 0.8, 0.6, 48.0, 0.0, 0.5};
    ok(escena_agregar_esfera(&e, (Vec3){0.0, 1.3, 0.0}, 0.25,
                             (Color){1.0, 1.0, 1.0}, segundo_cristal),
       "segundo cristal agregado en el trayecto hacia la luz");
    Color dos_cristales = De_Que_Color(&e, ojo, dir_a);
    ok(casi(dos_cristales.r, ambiente + 0.25 * difusa_total),
       "dos obstaculos al 50% multiplican la transmision de la luz");

    e.objetos[2].u.esfera.material.transparencia = 0.0;
    Color cristal_y_opaco = De_Que_Color(&e, ojo, dir_a);
    ok(casi(cristal_y_opaco.r, ambiente),
       "un obstaculo opaco anula la luz aunque antes haya un cristal");
    escena_liberar(&e);

    /* Totalmente transparente: no sombrea */
    ok(escena_sombra(&e, 1.0), "escena con obstaculo transparente");
    Color cristal = De_Que_Color(&e, ojo, dir_a);
    ok(casi(cristal.r, ambiente + difusa_total),
       "obstaculo transparente (transp = 1): sin sombra");
    escena_liberar(&e);

    ok(mitad.r > opaco.r && cristal.r > mitad.r,
       "el orden opaco < 50% < transparente es estricto");
}

/* ================================================================== */
/*  3. Transparencia NO refractiva (mezcla con tinte)                  */
/* ================================================================== */

static void prueba_transparencia(void)
{
    Escena e;
    Material m_cristal = {0.2, 0.0, 0.0, 48.0, 0.0, 0.0};

    printf("Transparencia sin refraccion:\n");

    /*
     * Escena sin luces: la iluminacion local de un objeto es
     *     local = base * ka * Ia
     * El fondo (0.8, 0.8, 0.8) se ve a traves del cristal y lo tiñe
     * con el color base del objeto.
     */
    escena_inicializar(&e);
    e.ojo = (Vec3){0.0, 0.0, -5.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.8, 0.8, 0.8};
    e.ambiente = 0.5;

    m_cristal.transparencia = 0.5;
    ok(escena_agregar_esfera(&e, (Vec3){0.0, 0.0, 2.0}, 1.0,
                             (Color){1.0, 0.5, 0.5}, m_cristal),
       "esfera de cristal rojizo agregada");

    /* Un rayo que golpea el centro de la esfera */
    Color visto = De_Que_Color(&e, (Vec3){0.0, 0.0, -5.0},
                               (Vec3){0.0, 0.0, 1.0});

    Color local = {1.0 * 0.2 * 0.5, 0.5 * 0.2 * 0.5, 0.5 * 0.2 * 0.5};
    Color detras = {0.8 * 1.0, 0.8 * 0.5, 0.8 * 0.5};  /* fondo * base */
    Color esperado;
    esperado.r = 0.5 * local.r + 0.5 * detras.r;
    esperado.g = 0.5 * local.g + 0.5 * detras.g;
    esperado.b = 0.5 * local.b + 0.5 * detras.b;

    printf("  visto    = %.4f %.4f %.4f\n", visto.r, visto.g, visto.b);
    printf("  esperado = %.4f %.4f %.4f\n",
           esperado.r, esperado.g, esperado.b);

    ok(color_casi(visto, esperado),
       "mezcla 50% local + 50% fondo teñido por el color base");
    ok(visto.r > visto.b,
       "el tinte colorea lo que se ve detras (el rojo se conserva mas)");
    escena_liberar(&e);

    /* Con transparencia = 1 solo queda lo de detrás, sin local */
    escena_inicializar(&e);
    e.ojo = (Vec3){0.0, 0.0, -5.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.8, 0.8, 0.8};
    e.ambiente = 0.5;
    m_cristal.transparencia = 1.0;
    escena_agregar_esfera(&e, (Vec3){0.0, 0.0, 2.0}, 1.0,
                          (Color){1.0, 0.5, 0.5}, m_cristal);

    Color puro = De_Que_Color(&e, (Vec3){0.0, 0.0, -5.0},
                              (Vec3){0.0, 0.0, 1.0});
    ok(casi(puro.r, 0.8) && casi(puro.g, 0.4) && casi(puro.b, 0.4),
       "transparencia = 1: solo el fondo teñido, sin componente local");
    escena_liberar(&e);
}

/* ================================================================== */
/*  4. Espejo: el rayo reflejado sale en la direccion correcta         */
/* ================================================================== */

static void prueba_espejo(void)
{
    Escena e;
    Material m_espejo = {0.4, 0.5, 0.8, 64.0, 1.0, 0.0};
    Material m_trasera = {0.5, 0.5, 0.0, 48.0, 0.0, 0.0};

    printf("Espejos:\n");

    /*
     * Esfera espejo delante del ojo y una esfera ROJA DETRAS del ojo
     * (z = -8). El rayo primario no puede alcanzarla: solo puede
     * verse si el rayo reflejado sale en la direccion contraria,
     * hacia el observador.
     *
     * Sin luces, el color de la esfera trasera es base * ka * Ia.
     */
    escena_inicializar(&e);
    e.ojo = (Vec3){0.0, 0.0, -5.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.0, 0.0, 0.0};
    e.ambiente = 0.5;

    ok(escena_agregar_esfera(&e, (Vec3){0.0, 0.0, 2.0}, 1.0,
                             (Color){1.0, 1.0, 1.0}, m_espejo),
       "esfera espejo agregada");
    ok(escena_agregar_esfera(&e, (Vec3){0.0, 0.0, -8.0}, 1.5,
                             (Color){1.0, 0.0, 0.0}, m_trasera),
       "esfera roja detras del ojo agregada");

    Color reflejo = De_Que_Color(&e, (Vec3){0.0, 0.0, -5.0},
                                 (Vec3){0.0, 0.0, 1.0});

    printf("  reflejo visto = %.4f %.4f %.4f\n",
           reflejo.r, reflejo.g, reflejo.b);

    ok(casi(reflejo.r, 1.0 * 0.5 * 0.5) &&
       casi(reflejo.g, 0.0) && casi(reflejo.b, 0.0),
       "el espejo devuelve la esfera roja detras del ojo");
    escena_liberar(&e);

    /* Espejo parcial (50%): mezcla de material propio y reflejo */
    m_espejo.reflejo = 0.5;
    m_espejo.ka = 0.4;
    escena_inicializar(&e);
    e.ojo = (Vec3){0.0, 0.0, -5.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.0, 0.0, 0.0};
    e.ambiente = 0.5;
    escena_agregar_esfera(&e, (Vec3){0.0, 0.0, 2.0}, 1.0,
                          (Color){0.0, 1.0, 0.0}, m_espejo);
    escena_agregar_esfera(&e, (Vec3){0.0, 0.0, -8.0}, 1.5,
                          (Color){1.0, 0.0, 0.0}, m_trasera);

    Color parcial = De_Que_Color(&e, (Vec3){0.0, 0.0, -5.0},
                                 (Vec3){0.0, 0.0, 1.0});

    /* local = (0, 1, 0) * 0.4 * 0.5 = (0, 0.2, 0) */
    double local_g = 1.0 * 0.4 * 0.5;
    double reflejo_r = 1.0 * 0.5 * 0.5;

    printf("  parcial  = %.4f %.4f %.4f\n",
           parcial.r, parcial.g, parcial.b);

    ok(casi(parcial.r, 0.5 * reflejo_r) && casi(parcial.g, 0.5 * local_g),
       "espejo al 50%: mitad reflejo y mitad material propio");
    escena_liberar(&e);

    /* Un mismo material combina iluminacion local, transmision y reflejo. */
    Material m_combinado = {0.5, 0.0, 0.0, 48.0, 0.5, 0.25};
    escena_inicializar(&e);
    e.ojo = (Vec3){0.0, 0.0, -5.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.2, 0.4, 0.8};
    e.ambiente = 0.4;
    escena_agregar_esfera(&e, (Vec3){0.0, 0.0, 2.0}, 1.0,
                          (Color){0.4, 0.8, 0.2}, m_combinado);
    escena_agregar_esfera(&e, (Vec3){0.0, 0.0, -8.0}, 1.5,
                          (Color){1.0, 0.0, 0.0}, m_trasera);

    Color combinado = De_Que_Color(&e, (Vec3){0.0, 0.0, -5.0},
                                   (Vec3){0.0, 0.0, 1.0});
   ok(casi(combinado.r, 0.14) && casi(combinado.g, 0.10) &&
       casi(combinado.b, 0.035),
       "un material mezcla correctamente iluminacion, transparencia y espejo");
    escena_liberar(&e);
}

/* ================================================================== */
/*  5. Limite de recursion: espejos enfrentados                        */
/* ================================================================== */

static void prueba_limite_recursion(void)
{
    Escena e;
    Material m_espejo = {0.3, 0.4, 0.9, 64.0, 1.0, 0.0};

    printf("Limite de recursion:\n");

    /*
     * Dos planos espejo enfrentados reflejarian el rayo infinitamente;
     * la profundidad maxima debe cortar la recursion sin bloquearse.
     */
    escena_inicializar(&e);
    e.ojo = (Vec3){0.0, 0.0, 3.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.1, 0.2, 0.3};
    e.ambiente = 0.5;

    Vec3 plano_a[4] = {
        {-5.0, -5.0, 0.0}, {5.0, -5.0, 0.0},
        {5.0, 5.0, 0.0}, {-5.0, 5.0, 0.0}
    };
    Vec3 plano_b[4] = {
        {-5.0, -5.0, 6.0}, {-5.0, 5.0, 6.0},
        {5.0, 5.0, 6.0}, {5.0, -5.0, 6.0}
    };

    ok(escena_agregar_poligono(&e, plano_a, 4, (Color){1.0, 1.0, 1.0},
                               m_espejo),
       "plano espejo A agregado");
    ok(escena_agregar_poligono(&e, plano_b, 4, (Color){1.0, 1.0, 1.0},
                               m_espejo),
       "plano espejo B agregado");

    Color c = De_Que_Color(&e, (Vec3){0.0, 0.0, 3.0}, (Vec3){0.0, 0.0, 1.0});

    ok(isfinite(c.r) && isfinite(c.g) && isfinite(c.b),
       "espejos enfrentados: la recursion termina con color valido");
    ok(c.r >= 0.0 && c.r <= 1.0 && c.g >= 0.0 && c.g <= 1.0 &&
       c.b >= 0.0 && c.b <= 1.0,
       "el color queda dentro de [0, 1]");
    escena_liberar(&e);
}

/* ================================================================== */
/*  main                                                               */
/* ================================================================== */

int main(void)
{
    prueba_lectura();
    printf("\n");
    prueba_sombra_transparente();
    printf("\n");
    prueba_transparencia();
    printf("\n");
    prueba_espejo();
    printf("\n");
    prueba_limite_recursion();

    printf("\n%s (%d fallos)\n", fallos ? "HAY FALLOS" : "TODO OK", fallos);
    return fallos ? 1 : 0;
}
