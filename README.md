# Proyecto 2 - Ray Tracing Básico

## Computación Gráfica

### Descripción

Este proyecto implementa un sistema básico de Ray Tracing
desarrollado en lenguaje C para Linux.

El programa genera una escena tridimensional compuesta
únicamente por esferas de diferentes tamaños, posiciones
y colores.

Se implementa iluminación difusa mediante la ley de Lambert, 
permitiendo representar las esferas con diferentes intensidades 
de color según la orientación de su superficie respecto a una fuente de luz.

No se implementan iluminación ambiental, sombras ni reflejos.

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
- vectores.c / vectores.h: operaciones vectoriales, cálculo 
de longitud y normalización.
- raytracing.c / raytracing.h: intersección rayo-esfera y 
cálculo de iluminación difusa mediante Lambert.
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
6. Se calcula el punto de intersección del rayo con la esfera.
7. Se obtiene el vector normal unitario de la superficie.
8. Se calcula el vector unitario hacia la fuente de luz.
9. Se aplica la ley de Lambert para obtener la intensidad difusa.
10. Se modifica el color original de la esfera según la intensidad calculada.
11. Se convierte el color al formato de 8 bits.
12. Se escribe la imagen en formato AVS.
13. Se muestra la imagen mediante ImageMagick.


## Iluminación difusa

El proyecto incorpora iluminación difusa mediante la ley de Lambert, permitiendo que las esferas presenten una apariencia tridimensional en lugar de colores sólidos.

Para calcular la iluminación, primero se determina el punto de intersección entre el rayo y la esfera:

P = E + tD

Posteriormente, se calcula el vector normal unitario de la superficie:

N = (P - C) / ||P - C||

Se obtiene el vector unitario dirigido hacia la fuente de luz:

L = (F - P) / ||F - P||

Finalmente, se aplica la ley de Lambert:

I = Kd * Ip * max(0, N · L)

Donde:

- Kd: coeficiente de reflexión difusa (0.8).
- Ip: intensidad de la fuente luminosa (1.0).
- N: vector normal unitario de la superficie.
- L: vector unitario hacia la fuente de luz.
- I: intensidad difusa resultante.

Se utiliza una fuente de luz ubicada en las coordenadas (-3.0, 4.0, -2.0).

La intensidad calculada se multiplica por los componentes RGB del color original de cada esfera.

Esto genera superficies más iluminadas cuando están orientadas hacia la fuente de luz y zonas oscuras cuando la luz no incide directamente.

No se implementan iluminación ambiental, atenuación por distancia ni múltiples fuentes luminosas.

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
- Se implementa iluminación difusa mediante la ley de Lambert.
- Se utiliza una fuente de luz puntual con posición e intensidad fijas.
- No se implementan iluminación ambiental, atenuación por 
distancia ni múltiples fuentes de luz.
- No se calculan sombras proyectadas ni reflejos especulares.
- Se utiliza memoria dinámica para almacenar las esferas.
- El programa comprueba errores de lectura y escritura.
- La estructura de objetos permite futuras ampliaciones.

## Integrantes

- Eliab Moises Hernandez Lopez
- Priscila Loaiza Sanchez
- Amanda Montero Zúñiga
