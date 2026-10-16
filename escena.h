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
 *   esfera      <cx> <cy> <cz> <radio> <r> <g> <b>
 *   ... (minimo 8 esferas)
 *   fin
 *
 * - Todos los valores son decimales (formato C, admite notacion 1e-3).
 * - Colores en el rango [0, 1].
 * - La ventana debe cumplir xmin < xmax y ymin < ymax.
 * - El radio debe ser mayor que cero.
 * - Las palabras clave son sensibles a minusculas.
 * - La linea "fin" es opcional; el final de archivo tambien cierra la escena.
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

/* Agrega una esfera a la escena. Devuelve 1 si tuvo exito, 0 si no. */
int escena_agregar_esfera(Escena *escena, Vec3 centro, double radio, Color color);

/* Agrega una fuente de luz a la escena */
int escena_agregar_luz(Escena *escena, Vec3 posicion, double intensidad);

/* Numero de esferas almacenadas (para escenas con mas tipos en el futuro). */
int escena_num_esferas(const Escena *escena);

/* Libera la memoria dinamica de la escena. Seguro de llamar mas de una vez. */
void escena_liberar(Escena *escena);

#endif /* ESCENA_H */
