# Proyecto 2 - Ray Tracing Básico

## Computación Gráfica

### Descripción

Este proyecto implementa un sistema básico de Ray Tracing
desarrollado en lenguaje C para Linux.

El programa genera una escena tridimensional compuesta por
esferas y polígonos (convexos y cóncavos) de diferentes
tamaños, posiciones, colores y materiales.

El modelo de iluminación es completo:

- Iluminación ambiental.
- Reflexión difusa de Lambert.
- Reflexión especular de Phong.
- Múltiples fuentes luminosas con atenuación por distancia.
- Sombras proyectadas.
- Materiales propios para cada objeto.

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
- objetos.h: estructuras de los objetos, colores, luces y materiales.
- vectores.c / vectores.h: operaciones vectoriales, cálculo 
de longitud y normalización.
- raytracing.c / raytracing.h: intersecciones y modelo de
iluminación completo (First_Intersection y De_Que_Color).
- poligonos.c / poligonos.h: geometría e intersección de polígonos.
- framebuffer.c / framebuffer.h: generación de archivos AVS.
- escena_materiales.txt: escena de prueba de sombras y materiales.
- pruebas/prueba_iluminacion.c: prueba del modelo de iluminación.
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

El archivo de escena contiene la información
necesaria para construir la escena.

Su formato es:

    resolucion ancho alto
    ojo x y z
    ventana xmin ymin xmax ymax
    fondo r g b
    ambiente Ia
    luz x y z intensidad
    esfera cx cy cz radio r g b [ka kd ks kn]
    poligono n x1 y1 z1 ... xn yn zn r g b [ka kd ks kn]
    material ka kd ks kn
    fin

Descripción de los parámetros:

- resolucion: dimensiones de la imagen.
- ojo: posición del observador en el espacio.
- ventana: límites del plano de proyección.
- fondo: color utilizado cuando no hay intersecciones.
- ambiente: intensidad de la iluminación ambiental, entre 0 y 1.
- luz: posición e intensidad de cada fuente de luz.
- esfera: centro, radio y color de cada esfera.
- poligono: número de vértices, sus coordenadas y el color.
  Admite polígonos convexos y cóncavos (mínimo 3 vértices).
- material: coeficientes del material del objeto anterior.
- fin: marca opcional de finalización del archivo.

Los componentes de color utilizan valores entre 0 y 1.

### Materiales

Cada objeto tiene su propio material, definido por cuatro
coeficientes que pueden escribirse al final de la línea del
objeto o, para reutilizarlos, en una línea `material` colocada
después del objeto al que aplican:

- ka: coeficiente de reflexión ambiental, entre 0 y 1.
- kd: coeficiente de reflexión difusa, entre 0 y 1.
- ks: coeficiente de reflexión especular, entre 0 y 1.
- kn: exponente de brillo, mayor que cero (típico: 8 - 256).

Si no se indican, se usan los valores por defecto
(0.3, 0.8, 0.6, 48).

Ejemplos:

    esfera -1.43 0.67 2.4 1.22 0.98 0.20 0.25 0.20 0.70 0.90 128
    poligono 4 -1 -1 -1  1 -1 -1  1 1 -1  -1 1 -1 1 0 0
    material 0.40 0.90 0.05 8

## Funcionamiento general

1. Se lee y valida el archivo de escena.
2. Se recorren los píxeles de la imagen.
3. Se genera un rayo desde el ojo hacia el centro de cada píxel.
4. `First_Intersection` busca la intersección más cercana
   con cualquier objeto (esfera o polígono).
5. Si no hay intersección se devuelve el color de fondo.
6. Se calcula el punto de intersección y su vector normal.
7. Se consulta el color y el material del objeto golpeado.
8. `De_Que_Color` aplica el modelo de iluminación completo.
9. Se convierte el color al formato de 8 bits.
10. Se escribe la imagen en formato AVS.
11. Se muestra la imagen mediante ImageMagick.

## Modelo de iluminación

Para cada punto de impacto P se suma la contribución de todas
las fuentes de luz más el término ambiental:

    I = ka * Ia + Σ_luz [ kd * Ip * Fatt * (N · L) ]
                    + Σ_luz [ ks * Ip * Fatt * (R · V)^kn ]

### Término ambiental

    Iamb = ka * Ia

No depende de la normal ni de la luz, por lo que nunca recibe
sombra. Ia se lee con la palabra clave `ambiente`.

### Reflexión difusa (Lambert)

    Idif = kd * Ip * Fatt * max(0, N · L)

- N: normal unitaria de la superficie en P.
- L: vector unitario desde P hacia la fuente de luz.
- kd: coeficiente difuso del material.
- Ip: intensidad de la fuente de luz.

La normal se orienta siempre hacia el lado del que llega el rayo.

### Atenuación por distancia

    Fatt = min(1, 1 / (C1 + C2 * d + C3 * d²))

con C1 = 1.0, C2 = 0.02 y C3 = 0.004, y d la distancia entre
el punto y la fuente de luz.

### Reflexión especular (Phong)

    Iesp = ks * Ip * Fatt * (R · V)^kn

- R: reflexión de L respecto de N,
  `R = 2 * (N · L) * N - L`.
- V: vector unitario desde P hacia el ojo.
- ks: coeficiente especular del material.
- kn: exponente de brillo; cuanto mayor es, más concentrado
  queda el brillo.

El término especular solo se calcula cuando N · L > 0 y
R · V > 0. El color del brillo es blanco, ya que representa
la luz de la fuente reflejada por la superficie.

### Sombras

Por cada fuente de luz se lanza un rayo de sombra desde el
punto de impacto, desplazado una pequeña cantidad a lo largo
de la normal para evitar auto-intersecciones, en la dirección
de la luz:

- Si algún objeto se encuentra entre el punto y la fuente,
  esa luz no aporta ni difusa ni especular.
- El término ambiental nunca se apaga.

De este modo los objetos proyectan sombras sobre el suelo y
sobre otros objetos.

### Materiales

Cada esfera y cada polígono tiene sus propios valores de
ka, kd, ks y kn, de modo que unos objetos responden de forma
mate y otros brillante con la misma iluminación.

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

## Pruebas

Para ejecutar la prueba del modelo de iluminación:

    make prueba

La prueba verifica la lectura de materiales y de la luz
ambiental, el funcionamiento de `First_Intersection` y
`De_Que_Color`, las sombras proyectadas, la reflexión
especular de Phong y las validaciones del archivo de escena.

## Consideraciones

- Todo el código del proyecto está desarrollado en C.
- Los algoritmos gráficos son implementaciones propias.
- No se utilizan bibliotecas gráficas para Ray Tracing.
- Se implementa iluminación ambiental y difusa mediante Lambert.
- Se implementa reflexión especular de Phong.
- Se utilizan múltiples fuentes de luz con atenuación por 
  distancia.
- Se calculan sombras proyectadas sobre esferas y polígonos.
- Cada objeto tiene su propio material (ka, kd, ks, kn).
- Se usa memoria dinámica para almacenar los objetos.
- El programa comprueba errores de lectura y escritura y no
  imprime valores de depuración.
- La estructura de objetos permite futuras ampliaciones.

## Integrantes

- Eliab Moises Hernandez Lopez
- Priscila Loaiza Sanchez
- Amanda Montero Zúñiga
