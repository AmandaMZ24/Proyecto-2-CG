# Proyecto 2 — Ray Tracing basico (C, Linux)

Esta version integra los archivos originales de escena de la persona 1,
los algoritmos de vectores/rayo-esfera/color de la persona 2 y un framebuffer
AVS de integracion para probar la salida (revisar con la persona 3 antes de unir cambios).

## Compilar y ejecutar

```sh
make clean && make
./raytracing escena.txt imagen.avs
magick imagen.avs -quality 100 imagen.jpg
magick display imagen.avs
```

Segun la instalacion de ImageMagick, usar `convert` en lugar de `magick`.
El programa no usa bibliotecas graficas: solo libreria matematica de C.

## Parametros

El archivo `escena.txt` define resolucion 1300x760, ojo, ventana,
color de fondo y diez esferas. El plano de la ventana es z=0.

## Modulo persona 2

`vectores.c`: suma, resta, producto por escalar y producto punto.
`raytracing.c`: `intersecar_esfera()` resuelve la cuadratica y devuelve
la interseccion positiva mas proxima; `trazar_pixel()` construye el rayo
y selecciona el objeto con el menor parametro t positivo, o devuelve el fondo.

Nota: no tiene iluminacion, sombras ni reflejos, tal como solicita el PDF.
No se incluyen los objetos compilados o ejecutables en el paquete fuente.
