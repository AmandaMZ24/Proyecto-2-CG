/*
 * escena.h
 *
 * Interfaz de carga y manejo de la escena.
 *
 * Formato del archivo de entrada (definido por el grupo):
 *
 *   # comentarios (lineas que empiezan con # se ignoran)
 *   resolucion  <ancho> <alto>
 *   ojo         <x> <y> <z>
 *   ventana     <xmin> <ymin> <xmax> <ymax>
 *   fondo       <r> <g> <b>
 *   ambiente    <Ia>                      (opcional, defecto 0.5)
 *   luz         <x> <y> <z> <intensidad>
 *   esfera      <cx> <cy> <cz> <radio> <r> <g> <b> [<ka> <kd> <ks> <kn>]
 *   poligono    <n> <x1> <y1> <z1> ... <xn> <yn> <zn> <r> <g> <b> [<ka> <kd> <ks> <kn>]
 *   material    <ka> <kd> <ks> <kn> [<reflejo> <transparencia>]
 *   espejo      <reflejo>                 (aplica al ultimo objeto)
 *   transparencia <transparencia>         (aplica al ultimo objeto)
 *   ... (minimo 8 esferas)
 *   fin
 *
 * - Todos los valores son decimales (formato C, admite notacion 1e-3).
 * - Colores en el rango [0, 1].
 * - La ventana debe cumplir xmin < xmax y ymin < ymax.
 * - El radio debe ser mayor que cero.
 * - Las palabras clave son sensibles a minusculas.
 * - La linea "fin" es opcional; el final de archivo tambien cierra la escena.
 *
 * MATERIALES
 *
 * Los cuatro coeficientes del material son opcionales:
 *
 *   ka   reflexion ambiental,  en [0, 1]
 *   kd   reflexion difusa,     en [0, 1]
 *   ks   reflexion especular,  en [0, 1]
 *   kn   exponente de brillo,  > 0 (tipico: 8 - 256)
 *
 * Ademas, cada objeto tiene dos propiedades opticas, ambas en [0, 1]
 * y con valor por defecto 0 (superficie opaca):
 *
 *   reflejo        0 = opaco, 1 = espejo perfecto. Fraccion del color
 *                  sustituida por el del rayo reflejado.
 *   transparencia  0 = opaco, 1 = totalmente transparente. Fraccion del
 *                  color sustituida por lo que se ve detras del objeto,
 *                  sin refraccion (el rayo no se desvia).
 *
 * Pueden escribirse al final de la linea del objeto (solo los cuatro
 * coeficientes) o, para reutilizar el mismo material en varios objetos,
 * en lineas separadas DESPUES del objeto al que aplican:
 *
 *   material <ka> <kd> <ks> <kn> [<reflejo> <transparencia>]
 *   espejo <reflejo>
 *   transparencia <transparencia>
 *
 * "material" con cuatro valores conserva el reflejo y la transparencia
 * que el objeto ya tuviera. "espejo" y "transparencia" solo cambian
 * su propia propiedad. Si no se indican, se usan los valores por
 * defecto (0.3, 0.8, 0.6, 48, 0, 0).
 */
#ifndef ESCENA_H
#define ESCENA_H

#include <stdio.h>

#include "objetos.h"

/* Numero minimo de esferas */
#define ESCENA_MIN_ESFERAS 8

/* Capacidad inicial del arreglo dinamico de objetos. */
#define ESCENA_CAP_INICIAL 16

/*
 * Maximo de vertices por poligono.
 * El lector actual admite hasta 64 tokens por linea.
 */
#define ESCENA_MAX_VERTICES 16

/*
 * Inicializa la escena vacia. Siempre debe llamarse (o bien usar
 * escena_leer, que lo hace internamente) antes de cualquier otra funcion.
 */
void escena_inicializar(Escena *escena);

/*
 * Lee y valida la escena desde `ruta`.
 * Devuelve 1 si todo esta correcto, 0 si hubo un error; en ese caso la
 * descripcion queda en escena_error(). La escena queda inicializada aunque
 * la lectura falle.
 */
int escena_leer(Escena *escena, const char *ruta);

/*
 * Lee y valida la escena desde un descriptor de archivo ya abierto.
 * Util para pruebas; el flujo NO se cierra aqui.
 */
int escena_leer_flujo(Escena *escena, FILE *flujo, const char *nombre);

/* Devuelve el ultimo error de lectura (cadena vacia si no hubo ninguno). */
const char *escena_error(const Escena *escena);

/* Llena `material` con los valores por defecto (0.3, 0.8, 0.6, 48). */
void material_defecto(Material *material);

/* Agrega una esfera a la escena. Devuelve 1 si tuvo exito, 0 si no. */
int escena_agregar_esfera(Escena *escena, Vec3 centro, double radio, Color color,
                          Material material);

/* Agrega un poligono a la escena. Devuelve 1 si tuvo exito, 0 si no. */
int escena_agregar_poligono(Escena *escena, const Vec3 *vertices, int num_vertices,
                            Color color, Material material);

/*
 * Asigna `material` al ultimo objeto agregado a la escena
 * (sintaxis de la palabra clave "material").
 * Devuelve 1 si tuvo exito, 0 si la escena esta vacia.
 */
int escena_aplicar_material(Escena *escena, Material material);

/* Agrega una fuente de luz a la escena */
int escena_agregar_luz(Escena *escena, Vec3 posicion, double intensidad);

/* Numero de esferas almacenadas (para escenas con mas tipos en el futuro). */
int escena_num_esferas(const Escena *escena);

/* Libera la memoria dinamica de la escena. Seguro de llamar mas de una vez. */
void escena_liberar(Escena *escena);

int escena_agregar_disco(Escena *escena, Vec3 centro, Vec3 normal, double radio, Color color, Material material);

int escena_agregar_elipse( Escena *escena, Vec3 foco1, Vec3 foco2, Vec3 normal, double distancia_maxima, Color color, Material material);

int escena_agregar_cilindro(Escena *escena, Vec3 ancla, Vec3 eje, double radio, double d_min, double d_max, Color color, Material material);

int escena_agregar_cono(Escena *escena, Vec3 vertice, Vec3 eje, double m, double n, double d_min, double d_max, Color color, Material material);

int escena_agregar_cuadratica(Escena *escena, Cuadratica superficie);

int escena_agregar_plano_corte(Escena *escena, PlanoCorte plano);

int escena_cargar_textura(Escena *escena, const char *ruta);

int escena_aplicar_textura(Escena *escena, int indice);

int escena_orientar_textura(Escena *escena, Vec3 g, Vec3 norte);
#endif /* ESCENA_H */
