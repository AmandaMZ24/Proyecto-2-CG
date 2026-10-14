# Proyecto 2 - Ray Tracing Básico

## Computación Gráfica

### Descripción

Este proyecto implementa un sistema básico de Ray Tracing
desarrollado en lenguaje C para Linux.

El programa genera una escena tridimensional compuesta
únicamente por esferas de diferentes tamaños, posiciones
y colores.

No se implementan modelos de iluminación, sombras ni
reflejos. Las esferas se representan mediante círculos
de colores sólidos.

La imagen se genera con una resolución de 1300 x 760
píxeles y se almacena en formato AVS.

## Requisitos

- Sistema operativo Linux.
- Compilador GCC.
- Herramienta Make.
- ImageMagick para visualizar y convertir imágenes.
- Biblioteca matemática estándar de C.

No se utilizan bibliotecas gráficas para implementar
los algoritmos de Ray Tracing.

## Estructura del proyecto

- main.c: coordina la ejecución del programa.
- escena.c / escena.h: lectura y validación de la escena.
- objetos.h: estructuras de los objetos y colores.
- vectores.c / vectores.h: operaciones vectoriales.
- raytracing.c / raytracing.h: intersección rayo-esfera.
- framebuffer.c / framebuffer.h: generación de archivos AVS.
- escena.txt: descripción de la escena.
- Makefile: compilación y ejecución del proyecto.

## Compilación

Desde la terminal de Linux, ubicarse en la carpeta
del proyecto y ejecutar:

    make clean
    make

Esto genera el ejecutable llamado raytracing.

## Ejecución

Para ejecutar el programa:

    make run

El programa lee escena.txt, calcula los colores de
los píxeles y genera el archivo imagen.avs.

Posteriormente, abre automáticamente la imagen
mediante ImageMagick.

También puede ejecutarse directamente:

    ./raytracing escena.txt imagen.avs

## Generación de JPG

Después de generar imagen.avs, ejecutar:

    make jpg

Esto convierte la imagen AVS a formato JPG utilizando
ImageMagick con calidad máxima (quality 100).

El archivo resultante es imagen.jpg.

## Archivo de entrada

El archivo escena.txt contiene la información
necesaria para construir la escena.

Su formato es:

    resolucion ancho alto
    ojo x y z
    ventana xmin ymin xmax ymax
    fondo r g b
    esfera cx cy cz radio r g b
    fin

Descripción de los parámetros:

- resolucion: dimensiones de la imagen.
- ojo: posición del observador en el espacio.
- ventana: límites del plano de proyección.
- fondo: color utilizado cuando no hay intersecciones.
- esfera: centro, radio y color de cada esfera.
- fin: marca opcional de finalización del archivo.

Los componentes de color utilizan valores entre 0 y 1.

La escena actual contiene 12 esferas y utiliza
una resolución de 1300 x 760 píxeles.

## Funcionamiento general

1. Se lee y valida el archivo de escena.
2. Se recorren los píxeles de la imagen.
3. Se genera un rayo desde el ojo hacia cada píxel.
4. Se calculan las intersecciones con las esferas.
5. Se selecciona la esfera visible más cercana.
6. Se asigna su color o el color de fondo.
7. Se convierte el color al formato de 8 bits.
8. Se escribe la imagen en formato AVS.
9. Se muestra la imagen mediante ImageMagick.

## Formato AVS

El archivo AVS se genera en formato binario.

Contiene:

- 4 bytes para el ancho de la imagen.
- 4 bytes para el alto de la imagen.
- 4 bytes por píxel en orden alfa, rojo, verde y azul.

Los valores de ancho y alto se almacenan en
orden de bytes big-endian.

El canal alfa se establece en 255, indicando
opacidad completa.

## Limpieza

Para eliminar el ejecutable y los archivos objeto:

    make clean

## Consideraciones

- Todo el código del proyecto está desarrollado en C.
- Los algoritmos gráficos son implementaciones propias.
- No se utilizan bibliotecas gráficas para Ray Tracing.
- No se implementa iluminación.
- Se utiliza memoria dinámica para almacenar las esferas.
- El programa comprueba errores de lectura y escritura.
- La estructura de objetos permite futuras ampliaciones.

## Integrantes

- Eliab Moises Hernandez Lopez
- [Nombre completo - Integrante 2]
- [Nombre completo - Integrante 3]