/*
 * prueba_iluminacion.c
 *
 * Prueba del modelo de iluminacion completo:
 *   - iluminacion ambiental
 *   - reflexion difusa de Lambert con atenuacion
 *   - reflexion especular de Phong: sumatoria((R.V)^kn * Ks * Ip * Fatt)
 *   - sombras proyectadas
 *   - materiales distintos por objeto
 *   - First_Intersection() y De_Que_Color()
 *
 * Compilar y ejecutar desde la raiz del proyecto con:  make prueba
 */
#include <stdio.h>
#include <math.h>

#include "../escena.h"
#include "../raytracing.h"
#include "../vectores.h"

static int fallos = 0;

static void ok(int cond, const char *msg)
{
    printf("%s %s\n", cond ? "[ok]  " : "[FALLO]", msg);
    if (!cond) fallos++;
}

/* Coeficientes de atenuacion usados por raytracing.c */
#define C2 0.02
#define C3 0.004

/* ================================================================== */
/*  1. Lectura de materiales y de la luz ambiental                     */
/* ================================================================== */
static void prueba_lectura(void)
{
    Escena e;

    if (!escena_leer(&e, "escena_materiales.txt")) {
        printf("[FALLO] no se pudo leer escena_materiales.txt: %s\n",
               escena_error(&e));
        fallos++;
        return;
    }

    printf("Lectura de la escena:\n");
    ok(fabs(e.ambiente - 0.5) < 1e-9, "luz ambiental leida (ambiente 0.5)");
    ok(e.num_luces == 3, "tres fuentes de luz en la escena");
    ok(e.num_objetos == 15, "15 objetos (3 poligonos + 12 esferas)");

    /* Suelo: material escrito al final de la linea */
    const Objeto *suelo = &e.objetos[0];
    ok(suelo->tipo == TIPO_POLIGONO, "objeto 0 es el suelo");
    ok(fabs(suelo->u.poligono.material.ka - 0.25) < 1e-9 &&
       fabs(suelo->u.poligono.material.kd - 0.70) < 1e-9 &&
       fabs(suelo->u.poligono.material.ks - 0.35) < 1e-9 &&
       fabs(suelo->u.poligono.material.kn - 16.0) < 1e-9,
       "suelo: material inline 0.25 0.70 0.35 16");

    /* Muro: material por la linea "material" posterior */
    const Objeto *muro = &e.objetos[1];
    ok(muro->tipo == TIPO_POLIGONO, "objeto 1 es el muro");
    ok(fabs(muro->u.poligono.material.ka - 0.40) < 1e-9 &&
       fabs(muro->u.poligono.material.kd - 0.90) < 1e-9 &&
       fabs(muro->u.poligono.material.ks - 0.05) < 1e-9 &&
       fabs(muro->u.poligono.material.kn - 8.0) < 1e-9,
       "muro: material de la linea \"material\" 0.40 0.90 0.05 8");

    /* Letra L: material inline */
    const Objeto *letra_l = &e.objetos[2];
    ok(fabs(letra_l->u.poligono.material.ks - 0.45) < 1e-9 &&
       fabs(letra_l->u.poligono.material.kn - 48.0) < 1e-9,
       "letra L: material inline");

    /* Esfera sin coeficientes -> valores por defecto */
    const Objeto *verde = &e.objetos[8];
    ok(verde->tipo == TIPO_ESFERA, "objeto 8 es la esfera verde");
    ok(fabs(verde->u.esfera.material.ka - MATERIAL_KA_DEFECTO) < 1e-9 &&
       fabs(verde->u.esfera.material.kd - MATERIAL_KD_DEFECTO) < 1e-9 &&
       fabs(verde->u.esfera.material.ks - MATERIAL_KS_DEFECTO) < 1e-9 &&
       fabs(verde->u.esfera.material.kn - MATERIAL_KN_DEFECTO) < 1e-9,
       "esfera verde: material por defecto 0.3 0.8 0.6 48");

    /* Esfera amarilla: sin especular */
    ok(fabs(e.objetos[7].u.esfera.material.ks) < 1e-9,
       "esfera amarilla: ks = 0 (sin brillo especular)");

    /* First_Intersection y De_Que_Color sobre la escena real */
    Vec3 origen = {0.0, 0.0, -5.0};
    Vec3 dir = {-3.2, -1.45, 8.0};     /* origen -> centro esfera roja */
    int idx = -1;
    double t = 0.0;

    ok(First_Intersection(&e, origen, dir, &idx, &t) && idx == 3 && t > 0.0,
       "First_Intersection golpea la esfera roja (indice 3)");

    Vec3 cielo = {0.0, 1.0, 0.0};
    ok(!First_Intersection(&e, origen, cielo, &idx, &t),
       "First_Intersection devuelve 0 cuando no hay impacto");

    Color fondo = De_Que_Color(&e, origen, cielo);
    ok(fabs(fondo.r - e.fondo.r) < 1e-9 &&
       fabs(fondo.g - e.fondo.g) < 1e-9 &&
       fabs(fondo.b - e.fondo.b) < 1e-9,
       "De_Que_Color devuelve el fondo cuando no hay impacto");

    /* Con las luces apagadas solo queda el termino ambiental */
    Escena sin_luz = e;
    sin_luz.num_luces = 0;
    Color ambiente = De_Que_Color(&sin_luz, origen, dir);
    ok(fabs(ambiente.r - 0.98 * 0.20 * 0.5) < 1e-9,
       "con las luces apagadas: color * ka * Ia");

    escena_liberar(&e);
}

/* ================================================================== */
/*  2. Sombras                                                         */
/* ================================================================== */
static void prueba_sombra(void)
{
    Escena e;
    escena_inicializar(&e);

    e.ojo = (Vec3){5.0, 10.0, -5.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.0, 0.0, 0.0};
    e.ambiente = 0.2;

    printf("Sombras:\n");

    /* Suelo horizontal, normal +Y */
    Vec3 suelo[4] = {
        {10.0, 0.0, -10.0},
        {-10.0, 0.0, -10.0},
        {-10.0, 0.0, 10.0},
        {10.0, 0.0, 10.0}
    };
    Material m_suelo = {0.5, 0.5, 0.0, 8.0};   /* ks = 0 */

    if (!escena_agregar_poligono(&e, suelo, 4, (Color){0.8, 0.8, 0.8}, m_suelo)) {
        printf("[FALLO] no se pudo agregar el suelo\n");
        fallos++;
        return;
    }

    /* Esfera que proyecta la sombra */
    Material m_esfera = {0.2, 0.8, 0.9, 64.0};
    escena_agregar_esfera(&e, (Vec3){0.0, 3.0, 0.0}, 1.0,
                          (Color){1.0, 1.0, 1.0}, m_esfera);

    /* Una sola luz justo encima de la esfera */
    escena_agregar_luz(&e, (Vec3){0.0, 6.0, 0.0}, 1.0);

    /* A = (0,0,0): exactamente debajo de la esfera -> SOMBRADO */
    Color a = De_Que_Color(&e, (Vec3){5.0, 10.0, -5.0}, (Vec3){-5.0, -10.0, 5.0});

    /* B = (5,0,0): lejos de la esfera -> ILUMINADO */
    Color b = De_Que_Color(&e, (Vec3){5.0, 10.0, -5.0}, (Vec3){0.0, -10.0, 5.0});

    printf("  A (sombra)   = %.4f %.4f %.4f\n", a.r, a.g, a.b);
    printf("  B (iluminado)= %.4f %.4f %.4f\n", b.r, b.g, b.b);

    double ambiente_A = 0.8 * 0.5 * 0.2;             /* color * ka * Ia */
    ok(fabs(a.r - ambiente_A) < 1e-12 &&
       fabs(a.g - ambiente_A) < 1e-12 &&
       fabs(a.b - ambiente_A) < 1e-12,
       "A queda en sombra: solo queda el termino ambiental");

    double d = sqrt(61.0);                            /* distancia a la luz */
    double NdotL = 6.0 / d;
    double Fatt = 1.0 / (1.0 + C2 * d + C3 * d * d);
    double esperado_b = 0.8 * 0.5 * 0.2 + 0.8 * (0.5 * 1.0 * Fatt * NdotL);

    printf("  B esperado   = %.4f\n", esperado_b);
    ok(fabs(b.r - esperado_b) < 1e-12,
       "B recibe difusa de Lambert con atenuacion por distancia");
    ok(b.r > a.r * 2.0, "la sombra oscurece claramente el punto A");

    /* El rayo de sombra de B no debe encontrar nada */
    Vec3 luz = {0.0, 6.0, 0.0};
    Vec3 dir_sombra = vec_normalizar(vec_restar(luz, (Vec3){5.0, 0.0, 0.0}));
    Vec3 origen_sombra = vec_sumar((Vec3){5.0, 0.0, 0.0},
                                   vec_escalar((Vec3){0.0, 1.0, 0.0}, 1e-4));
    int idx = -1;
    double t = 0.0;

    ok(!First_Intersection(&e, origen_sombra, dir_sombra, &idx, &t),
       "el rayo de sombra de B no encuentra nada");

    escena_liberar(&e);
}

/* ================================================================== */
/*  3. Reflexion especular de Phong                                    */
/* ================================================================== */
static void prueba_especular(void)
{
    Escena e;
    escena_inicializar(&e);

    e.ojo = (Vec3){0.0, 0.0, -5.0};
    e.ancho = 4;
    e.alto = 4;
    e.ventana_xmin = -1; e.ventana_xmax = 1;
    e.ventana_ymin = -1; e.ventana_ymax = 1;
    e.fondo = (Color){0.0, 0.0, 0.0};
    e.ambiente = 0.2;

    printf("Reflexion especular:\n");

    Material m = {0.1, 0.5, 0.5, 64.0};
    escena_agregar_esfera(&e, (Vec3){0.0, 0.0, 0.0}, 1.0,
                          (Color){1.0, 1.0, 1.0}, m);

    /* Luz en la posicion del ojo: en el polo frontal R = V */
    escena_agregar_luz(&e, (Vec3){0.0, 0.0, -5.0}, 1.0);

    Vec3 ojo = {0.0, 0.0, -5.0};
    Vec3 dir = {0.0, 0.0, 4.0};          /* golpea (0,0,-1) */

    Color con_ks = De_Que_Color(&e, ojo, dir);

    e.objetos[0].u.esfera.material.ks = 0.0;
    Color sin_ks = De_Que_Color(&e, ojo, dir);
    e.objetos[0].u.esfera.material.ks = 0.5;

    printf("  con Ks=0.5 = %.4f   sin Ks = %.4f\n", con_ks.r, sin_ks.r);

    double Fatt = 1.0 / (1.0 + C2 * 4.0 + C3 * 16.0);
    double esperado_sin = 1.0 * 0.1 * 0.2 + 1.0 * (0.5 * 1.0 * Fatt * 1.0);
    double esperado_con = esperado_sin + 1.0 * (0.5 * 1.0 * Fatt * 1.0);

    ok(fabs(sin_ks.r - esperado_sin) < 1e-12,
       "sin Ks: ambiente + difusa");
    ok(fabs(con_ks.r - esperado_con) < 1e-12,
       "con Ks: ambiente + difusa + Ks*Ip*Fatt*(R.V)^kn");
    ok(con_ks.r > sin_ks.r + 0.1, "el brillo especular se nota claramente");

    /* En el punto exacto del reflejo R.V = 1: el exponente no cambia nada */
    e.objetos[0].u.esfera.material.kn = 4.0;
    Color kn_bajo = De_Que_Color(&e, ojo, dir);
    e.objetos[0].u.esfera.material.kn = 256.0;
    Color kn_alto = De_Que_Color(&e, ojo, dir);
    e.objetos[0].u.esfera.material.kn = 64.0;

    ok(fabs(kn_bajo.r - con_ks.r) < 1e-12 &&
       fabs(kn_alto.r - con_ks.r) < 1e-12,
       "con R.V = 1 el exponente kn no cambia el resultado");

    /* Fuera del punto de reflejo si importa */
    Vec3 dir2 = {0.5, 0.0, 4.134};       /* 30 grados: R.V queda en (0,1) */
    e.objetos[0].u.esfera.material.kn = 4.0;
    Color k4 = De_Que_Color(&e, ojo, dir2);
    e.objetos[0].u.esfera.material.kn = 256.0;
    Color k256 = De_Que_Color(&e, ojo, dir2);
    e.objetos[0].u.esfera.material.kn = 64.0;

    ok(k4.r > k256.r, "mayor kn => brillo mas concentrado");

    /* Mismo material, distinto kd => distinta respuesta */
    e.objetos[0].u.esfera.material.kd = 0.1;
    Color kd_bajo = De_Que_Color(&e, ojo, dir);
    e.objetos[0].u.esfera.material.kd = 0.9;
    Color kd_alto = De_Que_Color(&e, ojo, dir);

    ok(kd_alto.r > kd_bajo.r, "mayor kd => superficie mas luminosa");

    escena_liberar(&e);
}

/* ================================================================== */
/*  4. Validaciones del parser de materiales                           */
/* ================================================================== */
static void escribir_escena_mala(const char *ruta, const char *material,
                                 int material_suelto)
{
    FILE *f = fopen(ruta, "w");
    if (f == NULL) return;

    fprintf(f, "resolucion 10 10\nojo 0 0 -5\nventana -1 -1 1 1\nfondo 0 0 0\n");
    if (material_suelto) {
        fprintf(f, "%s\n", material);
    } else {
        fprintf(f, "esfera 0 0 2 1 1 0 0 %s\n", material);
    }
    for (int i = 1; i < 8; i++) {
        fprintf(f, "esfera %d 0 2 1 1 0 0\n", i);
    }
    fprintf(f, "fin\n");
    fclose(f);
}

static void prueba_parser(void)
{
    Escena e;

    printf("Validaciones:\n");

    escribir_escena_mala("/tmp/prueba_ks.txt", "0.2 0.8 0.9 -4", 0);
    ok(!escena_leer(&e, "/tmp/prueba_ks.txt"), "kn negativo rechazado");
    escena_liberar(&e);

    escribir_escena_mala("/tmp/prueba_ka.txt", "0.2 0.8 1.5 10", 0);
    ok(!escena_leer(&e, "/tmp/prueba_ka.txt"), "ks fuera de [0,1] rechazado");
    escena_liberar(&e);

    escribir_escena_mala("/tmp/prueba_mat.txt", "material 0.5 0.5 0.5 8", 1);
    ok(!escena_leer(&e, "/tmp/prueba_mat.txt"),
       "\"material\" sin objeto previo rechazado");
    escena_liberar(&e);

    remove("/tmp/prueba_ks.txt");
    remove("/tmp/prueba_ka.txt");
    remove("/tmp/prueba_mat.txt");
}

int main(void)
{
    prueba_lectura();
    printf("\n");
    prueba_sombra();
    printf("\n");
    prueba_especular();
    printf("\n");
    prueba_parser();

    printf("\n%s (%d fallos)\n", fallos ? "HAY FALLOS" : "TODO OK", fallos);
    return fallos ? 1 : 0;
}
