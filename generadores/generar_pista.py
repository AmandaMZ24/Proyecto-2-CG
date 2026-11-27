from pathlib import Path
from math import sqrt
import math

ORIGINAL = Path("escenas_finales/mcqueen_importado.txt")
SALIDA = Path("escenas_finales/cars_meta.txt")

lineas = ORIGINAL.read_text().splitlines()

# Encontrar el poligono correspondiente al suelo
indice_suelo = next(
    i for i in range(len(lineas) - 1, -1, -1)
    if lineas[i].startswith("poligono 4 ")
)

datos = lineas[indice_suelo].split()

vertices = [
    tuple(map(float, datos[2 + i * 3:5 + i * 3]))
    for i in range(4)
]

A, B, C, D = vertices


def restar(a, b):
    return tuple(x - y for x, y in zip(a, b))


def cruz(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    )


def normalizar(v):
    longitud = sqrt(sum(x * x for x in v))
    return tuple(x / longitud for x in v)


normal = normalizar(
    cruz(restar(B, A), restar(C, A))
)

# Orientar la normal hacia arriba
if normal[1] < 0:
    normal = tuple(-x for x in normal)

nx, ny, nz = normal

d = -sum(a * b for a, b in zip(normal, A))


def altura(x, z):
    return -(nx * x + nz * z + d) / ny


def punto(x, z, h=0):
    return (
        x + nx * h,
        altura(x, z) + ny * h,
        z + nz * h
    )


def formato(valores):
    return " ".join(f"{v:.8f}" for v in valores)


objetos = []


def agregar(tipo, valores):
    objetos.append(f"{tipo} {formato(valores)}")


def material(ka, kd, ks, kn, reflejo=0, transparencia=0):
    agregar(
        "material",
        (ka, kd, ks, kn, reflejo, transparencia)
    )


def poligono(vertices, color):
    valores = [
        componente
        for vertice in vertices
        for componente in vertice
    ]

    objetos.append(
        f"poligono {len(vertices)} "
        f"{formato(valores)} {formato(color)}"
    )


def disco(centro, eje, radio, color):
    agregar("disco", (*centro, *eje, radio, *color))


def cilindro(centro, eje, radio, minimo, maximo, color):
    agregar(
        "cilindro",
        (*centro, *eje, radio, minimo, maximo, *color)
    )


def cono(vertice, eje, m, n, minimo, maximo, color):
    agregar(
        "cono",
        (*vertice, *eje, m, n, minimo, maximo, *color)
    )


def esfera(centro, radio, color):
    agregar("esfera", (*centro, radio, *color))

def cuadratica(coeficientes, color):
    agregar(
        "cuadratica",
        (*coeficientes, *color)
    )


def corte(eje, d):
    agregar("corte", (*eje, d))


def elipse(foco1, foco2, eje, suma_distancias, color):
    agregar(
        "elipse",
        (*foco1, *foco2, *eje, suma_distancias, *color)
    )


def producto_punto(a, b):
    return sum(x * y for x, y in zip(a, b))

def paraboloide(vertice, eje, k, color):

    nx, ny, nz = eje
    cx, cy, cz = vertice

    A = 1 - nx * nx
    B = 1 - ny * ny
    C = 1 - nz * nz

    D = -2 * nx * ny
    E = -2 * nx * nz
    F = -2 * ny * nz

    G = -2*A*cx - D*cy - E*cz - k*nx
    H = -2*B*cy - D*cx - F*cz - k*ny
    I = -2*C*cz - E*cx - F*cy - k*nz

    J = (
        A*cx*cx +
        B*cy*cy +
        C*cz*cz +
        D*cx*cy +
        E*cx*cz +
        F*cy*cz +
        k*producto_punto(eje, vertice)
    )

    cuadratica(
        (A, B, C, D, E, F, G, H, I, J),
        color
    )
# ==================================================
# LINEA DE META
# ==================================================

objetos.append("# LINEA DE META")

for fila in range(2):
    for columna in range(32):

        x = -11.2 + columna * 0.7
        z = 2.2 + fila * 0.65

        color = (
            (0.95, 0.95, 0.95)
            if (fila + columna) % 2 == 0
            else (0.025, 0.025, 0.025)
        )

        poligono(
            [
                punto(x, z, 0.005),
                punto(x + 0.7, z, 0.005),
                punto(x + 0.7, z + 0.65, 0.005),
                punto(x, z + 0.65, 0.005)
            ],
            color
        )

        material(0.65, 0.8, 0.35, 32, 0.25)


# ==================================================
# CONOS DE SEGURIDAD
# ==================================================

def cono_seguridad(x, z):

    naranja = (1.0, 0.28, 0.025)
    blanco = (0.95, 0.95, 0.95)
    negro = (0.06, 0.06, 0.06)

    eje_abajo = tuple(-v for v in normal)

    vertice = punto(x, z, 0.85)

    # Parte superior naranja
    cono(
        vertice, eje_abajo,
        3, 1, 0, 0.30,
        naranja
    )

    material(0.6, 0.8, 0.25, 32)

    # Franja blanca
    cono(
        vertice, eje_abajo,
        3, 1, 0.30, 0.47,
        blanco
    )

    material(0.7, 0.8, 0.25, 32)

    # Parte inferior naranja
    cono(
        vertice, eje_abajo,
        3, 1, 0.47, 0.80,
        naranja
    )

    material(0.6, 0.8, 0.25, 32)

    # Base negra
    cilindro(
        punto(x, z, 0.005),
        normal,
        0.35, 0, 0.06,
        negro
    )

    disco(
        punto(x, z, 0.065),
        normal,
        0.35,
        negro
    )


objetos.append("# CONOS DE SEGURIDAD")

for x, z in [
    (-6.8, 3.0),
    (-7.5, 5.0),
    (-8.5, 8.0),
    (6.8, 3.0),
    (7.5, 5.0),
    (8.5, 8.0)
]:
    cono_seguridad(x, z)


# ==================================================
# BARRILES DINOCO
# ==================================================

def barril(x, z):

    azul = (0.04, 0.45, 0.85)
    metal = (0.65, 0.70, 0.75)

    base = punto(x, z, 0.01)

    # Cuerpo principal
    cilindro(
        base, normal,
        0.48, 0, 1.25,
        azul
    )

    material(0.6, 0.8, 0.5, 48)

    objetos.append("textura 4")

    # Tapa superior
    disco(
        punto(x, z, 1.26),
        normal,
        0.49,
        azul
    )

    material(0.6, 0.8, 0.5, 48)

    # Anillos metalicos
    for h in [0.04, 0.30, 0.95, 1.20]:

        cilindro(
            punto(x, z, h),
            normal,
            0.50, 0, 0.055,
            metal
        )

        material(0.4, 0.7, 0.8, 64)


objetos.append("# BARRILES")

barril(-6.2, 1.8)
barril(-8.0, 6.0)
barril(7.8, 5.0)

# ==================================================
# SEMAFORO DE CARRERAS
# ==================================================

objetos.append("# SEMAFORO DE CARRERAS")

x, z = -10.5, 14.0

negro = (0.035, 0.035, 0.04)
metal = (0.55, 0.58, 0.62)

# Poste
cilindro(
    punto(x, z),
    normal,
    0.10, 0, 4.0,
    metal
)

material(0.5, 0.7, 0.8, 64, 0.20)

# Base
disco(
    punto(x, z, 0.02),
    normal,
    0.40,
    negro
)

# Cuerpo del semaforo
cilindro(
    punto(x, z, 2.25),
    normal,
    0.34, 0, 1.75,
    negro
)

material(0.5, 0.7, 0.3, 32)

disco(
    punto(x, z, 4.0),
    normal,
    0.34,
    negro
)

# Luces
for h, color in [
    (3.65, (1.0, 0.015, 0.015)),
    (3.12, (1.0, 0.85, 0.015)),
    (2.59, (0.015, 1.0, 0.08))
]:

    esfera(
        punto(x, z - 0.32, h),
        0.23,
        color
    )

    material(1.0, 0.4, 0.85, 64)

# ==================================================
# GLOBO PUBLICITARIO
# ==================================================

objetos.append("# GLOBO PUBLICITARIO")

x, z = 9.5, 13.0

# Esfera con textura
esfera(
    punto(x, z, 1.85),
    0.70,
    (1.0, 1.0, 1.0)
)

material(0.7, 0.8, 0.4, 48)

objetos.append("textura 5")

# Soporte
cilindro(
    punto(x, z),
    normal,
    0.065, 0, 0.85,
    (0.35, 0.35, 0.38)
)

material(0.5, 0.7, 0.5, 32)

# Base
disco(
    punto(x, z, 0.015),
    normal,
    0.35,
    (0.15, 0.15, 0.15)
)

# ==================================================
# EXTENSION DEL ASFALTO
# ==================================================

poligono(
    [
        punto(-60, -20, -0.01),
        punto(60, -20, -0.01),
        punto(60, 100, -0.01),
        punto(-60, 100, -0.01)
    ],
    (0.27, 0.28, 0.30)
)

material(0.45, 0.65, 0.85, 80, 0.55)
objetos.append("textura 6")

# ==================================================
# MURO DEL ESTADIO
# ==================================================

poligono(
    [
        punto(-15, 18, 0),
        punto(15, 18, 0),
        punto(15, 18, 1.5),
        punto(-15, 18, 1.5)
    ],
    (0.75, 0.76, 0.78)
)

material(0.6, 0.8, 0.15, 16)

# Franja roja del muro
poligono(
    [
        punto(-15, 17.99, 1.1),
        punto(15, 17.99, 1.1),
        punto(15, 17.99, 1.5),
        punto(-15, 17.99, 1.5)
    ],
    (0.85, 0.04, 0.04)
)

material(0.65, 0.8, 0.1, 16)

# ==================================================
# TROFEO COPA PISTON
# ==================================================

objetos.append("# TROFEO COPA PISTON")

x, z = -4.8, 0.5

oro = (0.95, 0.65, 0.08)
oro_claro = (1.0, 0.82, 0.25)
oro_oscuro = (0.65, 0.38, 0.04)
negro = (0.07, 0.06, 0.05)

# Base inferior
cilindro(
    punto(x, z, 0.015),
    normal,
    0.43, 0, 0.13,
    negro
)

material(0.6, 0.7, 0.4, 32)

disco(
    punto(x, z, 0.145),
    normal,
    0.43,
    negro
)

# Base dorada
cilindro(
    punto(x, z, 0.145),
    normal,
    0.30, 0, 0.12,
    oro
)

material(0.5, 0.7, 0.9, 64, 0.15)

disco(
    punto(x, z, 0.265),
    normal,
    0.30,
    oro
)

# Cuello del trofeo
cilindro(
    punto(x, z, 0.265),
    normal,
    0.12, 0, 0.38,
    oro
)

material(0.5, 0.7, 1.0, 25, 0.20)

# Copa exterior
vertice = punto(x, z, 0.60)
borde = punto(x, z, 1.35)

paraboloide(
    vertice,
    normal,
    0.28,
    oro
)

# Eliminar la parte superior infinita
corte(
    normal,
    -producto_punto(normal, borde)
)

material(0.55, 0.75, 1.0, 25, 0.20)

# Interior de la copa
vertice_interior = punto(x, z, 0.70)

paraboloide(
    vertice_interior,
    normal,
    0.28 * 0.75 / 0.65,
    oro_oscuro
)

corte(
    normal,
    -producto_punto(normal, borde)
)

material(0.4, 0.65, 0.8, 64, 0.10)

# Borde superior
cilindro(
    punto(x, z, 1.32),
    normal,
    0.46, 0, 0.04,
    oro_claro
)

material(0.6, 0.75, 1.0, 25, 0.20)

# ==================================================
# LETRERO OVALADO DE COMPETICION
# ==================================================

objetos.append("# LETRERO OVALADO")

x, z = 4.5, 0.8

oro = (1.0, 0.75, 0.08)
rojo = (0.85, 0.035, 0.025)
negro = (0.035, 0.035, 0.045)

# Base circular
disco(
    punto(x, z, 0.025),
    normal,
    0.42,
    negro
)

# Poste metalico
cilindro(
    punto(x, z),
    normal,
    0.075, 0, 1.2,
    (0.55, 0.58, 0.65)
)

material(0.5, 0.7, 0.85, 64, 0.25)

# Centro del letrero
cx, cy, cz = punto(x, z, 1.45)

# Marco eliptico dorado
elipse(
    (cx - 0.38, cy, cz),
    (cx + 0.38, cy, cz),
    (0, 0, -1),
    1.65,
    oro
)

material(0.7, 0.8, 0.9, 64, 0.15)

# Interior negro
elipse(
    (cx - 0.38, cy, cz - 0.015),
    (cx + 0.38, cy, cz - 0.015),
    (0, 0, -1),
    1.45,
    negro
)

material(0.8, 0.6, 0.1, 16)

# Rayo decorativo
vertices_rayo = [
    (-0.08, 0.40),
    (0.32, 0.40),
    (0.06, 0.06),
    (0.30, 0.06),
    (-0.32, -0.40),
    (-0.06, -0.04),
    (-0.30, -0.04)
]

# Triangulacion del rayo
for a, b, c in [
    (0, 1, 2),
    (0, 2, 5),
    (0, 5, 6),
    (2, 3, 5),
    (3, 4, 5)
]:

    poligono(
        [
            (cx + vertices_rayo[i][0],
             cy + vertices_rayo[i][1],
             cz - 0.03)
            for i in (a, b, c)
        ],
        rojo
    )

    material(1.0, 0.6, 0.5, 32)

# ==================================================
# CABINA DE COMENTARISTAS
# ==================================================

objetos.append("# CABINA DE COMENTARISTAS")

x, z = -6.5, 16.5

metal = (0.30, 0.33, 0.38)
cristal = (0.88, 0.95, 1.0)

# Dimensiones de la cabina
ancho = 2.35
profundidad = 3.0
altura_inferior = 1.50
altura_superior = 3.70

# Plataforma inferior
poligono(
    [
        punto(x - ancho, z, altura_inferior),
        punto(x + ancho, z, altura_inferior),
        punto(x + ancho, z + profundidad, altura_inferior),
        punto(x - ancho, z + profundidad, altura_inferior)
    ],
    metal
)

material(0.6, 0.7, 0.5, 32)

# Pared posterior
poligono(
    [
        punto(x - ancho, z + profundidad, altura_inferior),
        punto(x + ancho, z + profundidad, altura_inferior),
        punto(x + ancho, z + profundidad, altura_superior),
        punto(x - ancho, z + profundidad, altura_superior)
    ],
    (0.65, 0.72, 0.80)
)

material(1.0, 0.6, 0.1, 16)

# Ventanas transparentes
for dx in [-ancho, 0]:

    poligono(
        [
            punto(x + dx, z, altura_inferior),
            punto(x + dx + ancho, z, altura_inferior),
            punto(x + dx + ancho, z, altura_superior),
            punto(x + dx, z, altura_superior)
        ],
        cristal
    )

    material(0.15, 0.20, 0.85, 64, 0.05, 0.88)

# Marcos metalicos verticales
for dx in [-ancho, 0, ancho]:

    cilindro(
        punto(x + dx, z - 0.015, altura_inferior),
        normal,
        0.045, 0,
        altura_superior - altura_inferior,
        metal
    )

    material(0.6, 0.7, 0.8, 48)

# Marcos metalicos horizontales
for h in [altura_inferior, altura_superior]:

    cilindro(
        punto(x - ancho, z - 0.015, h),
        (1, 0, 0),
        0.055, 0, ancho * 2,
        metal
    )

    material(0.6, 0.7, 0.8, 48)

# Techo
poligono(
    [
        punto(x - ancho - 0.15, z - 0.15, altura_superior),
        punto(x + ancho + 0.15, z - 0.15, altura_superior),
        punto(x + ancho + 0.15, z + profundidad + 0.15, altura_superior),
        punto(x - ancho - 0.15, z + profundidad + 0.15, altura_superior)
    ],
    metal
)

material(0.6, 0.7, 0.5, 32)

# ==================================================
# GRADAS DEL ESTADIO
# ==================================================

objetos.append("# GRADAS DEL ESTADIO")

for fila in range(5):

    z = 19.0 + fila * 0.85
    h = 1.5 + fila * 0.35

    color = (
        (0.32, 0.34, 0.38)
        if fila % 2 == 0
        else (0.55, 0.57, 0.60)
    )

    # Superficie horizontal
    poligono(
        [
            punto(-18, z, h),
            punto(18, z, h),
            punto(18, z + 0.85, h),
            punto(-18, z + 0.85, h)
        ],
        color
    )

    material(0.65, 0.8, 0.1, 16)

    # Superficie vertical
    poligono(
        [
            punto(-18, z, h - 0.35),
            punto(18, z, h - 0.35),
            punto(18, z, h),
            punto(-18, z, h)
        ],
        color
    )

    material(0.65, 0.8, 0.1, 16)

# ==================================================
# PUBLICO DEL ESTADIO
# ==================================================

objetos.append("# PUBLICO DEL ESTADIO")

for fila in range(5):

    z = 18.95 + fila * 0.85
    h = 1.50 + fila * 0.35

    poligono(
        [
            punto(-18, z, h),
            punto(18, z, h),
            punto(18, z, h + 0.48),
            punto(-18, z, h + 0.48)
        ],
        (1.0, 1.0, 1.0)
    )

    material(0.85, 0.65, 0.05, 8)

    objetos.append("textura 8")

# ==================================================
# PUBLICIDAD DEL ESTADIO
# ==================================================

poligono(
    [
        punto(-6, 17.95, 0.25),
        punto(6, 17.95, 0.25),
        punto(6, 17.95, 1.05),
        punto(-6, 17.95, 1.05)
    ],
    (1.0, 1.0, 1.0)
)

material(0.85, 0.7, 0.05, 8)

objetos.append("textura 7")


# ==================================================
# TORRES DE ILUMINACION
# ==================================================

objetos.append("# REFLECTORES DEL ESTADIO")

metal = (0.35, 0.38, 0.45)
blanco = (1.0, 0.98, 0.90)
negro = (0.04, 0.04, 0.05)


def torre_luces(x, z):

    # Poste principal
    cilindro(
        punto(x, z),
        normal,
        0.16, 0, 5.2,
        metal
    )

    material(0.5, 0.7, 0.9, 64, 0.35)

    # Base circular
    disco(
        punto(x, z, 0.03),
        normal,
        0.50,
        metal
    )

    # Panel rectangular
    poligono(
        [
            punto(x - 1.5, z, 4.3),
            punto(x + 1.5, z, 4.3),
            punto(x + 1.5, z, 5.5),
            punto(x - 1.5, z, 5.5)
        ],
        negro
    )

    material(0.6, 0.7, 0.3, 32)

    # 18 bombillas por torre
    for fila in range(3):

        for columna in range(6):

            dx = -1.1 + columna * 0.44
            h = 4.55 + fila * 0.35

            esfera(
                punto(x + dx, z - 0.20, h),
                0.14,
                blanco
            )

            material(1.0, 0.5, 0.8, 64)


torre_luces(-10, 17)
torre_luces(10, 17)

# ==================================================
# MARCADOR GIGANTE
# ==================================================

objetos.append("# MARCADOR GIGANTE")

poligono(
    [
        punto(-5.5, 18, 4.0),
        punto(5.5, 18, 4.0),
        punto(5.5, 18, 5.4),
        punto(-5.5, 18, 5.4)
    ],
    (1.0, 1.0, 1.0)
)

material(1.0, 0.2, 0.05, 8)

objetos.append("textura 9")

# ==================================================
# ESTRUCTURA DEL ESTADIO
# ==================================================

objetos.append("# COLUMNAS DEL ESTADIO")

for x in [-15, -7.5, 0, 7.5, 15]:

    cilindro(
        punto(x, 18.7, 1.5),
        normal,
        0.065, 0, 2.2,
        (0.45, 0.48, 0.55)
    )

    material(0.5, 0.7, 0.8, 48, 0.15)


# Barandal horizontal
for h in [1.55, 3.65]:

    cilindro(
        punto(-18, 18.7, h),
        (1, 0, 0),
        0.045, 0, 36,
        (0.65, 0.68, 0.72)
    )

    material(0.5, 0.7, 0.85, 64, 0.25)

# ==================================================
# DESTELLOS DE LUZ
# ==================================================

def destello(cx, cy, cz, radio, color):

    # Rayos en cuatro direcciones
    for angulo in [0, 45, 90, 135]:

        a = math.radians(angulo)

        dx = math.cos(a)
        dy = math.sin(a)

        largo = radio if angulo % 90 == 0 else radio * 0.55
        ancho = radio * 0.055

        poligono(
            [
                (cx - dx*largo, cy - dy*largo, cz),
                (cx - dy*ancho, cy + dx*ancho, cz),
                (cx + dx*largo, cy + dy*largo, cz),
                (cx + dy*ancho, cy - dx*ancho, cz)
            ],
            color
        )

        material(1.0, 0.35, 0.0, 16, 0, 0.05)

        cz -= 0.002

for x in [-10, 10]:

    for dx in [-0.66, 0.66]:

        centro = punto(
            x + dx,
            16.25,
            4.90
        )

        destello(
            *centro,
            1.0,
            (1.0, 0.98, 0.89)
        )

# ==================================================
# FLASHES DEL PUBLICO
# ==================================================

for x, fila in [
    (-12, 2),
    (-8, 4),
    (-3, 1),
    (2, 3),
    (7, 2),
    (12, 4)
]:

    z = 18.65 + fila * 0.85
    h = 1.75 + fila * 0.35

    centro = punto(x, z, h)

    destello(
        *centro,
        0.22,
        (1.0, 1.0, 1.0)
    )
# ==================================================
# CONFIGURACION DE LA ESCENA
# ==================================================

resultado = []

for linea in lineas:

    if linea.startswith("resolucion "):
        linea = "resolucion 800 468"
    
    elif linea.startswith("ventana "):
        linea = "ventana -6.5 -3.8 6.5 3.8"

    elif linea.startswith("antialiasing "):
        linea = "antialiasing 0"

    elif linea.startswith("fondo "):
        linea = "fondo 0.035 0.055 0.12"
    
    elif linea.startswith("ambiente "):
        linea = "ambiente 0.70"
    
    elif linea.startswith("luz -6 "):
        linea = "luz -6 9 -8 1.0"
    
    elif linea.startswith("luz 6 "):
        linea = "luz 6 7 -4 0.90"

    elif linea.strip() == "fin":
        continue

    resultado.append(linea)

# Aplicar textura al suelo
resultado.insert(indice_suelo + 2, "textura 6")
resultado[indice_suelo + 1] = (
    "material 0.45 0.65 0.85 80 0.55 0"
)

# Encontrar la ultima textura original de McQueen
indice_texturas = max(
    i for i, linea in enumerate(resultado)
    if linea.startswith("textura_cargar ")
)

# Cargar todas las texturas adicionales antes de utilizarlas
resultado[indice_texturas + 1:indice_texturas + 1] = [
    "textura_cargar texturas/cars/dinoco_corregido.avs",
    "textura_cargar texturas/cars/rusteze_corregido.avs",
    "textura_cargar texturas/cars/asfalto_mojado.avs",
    "textura_cargar texturas/cars/piston_cup.avs",
    "textura_cargar texturas/cars/publico.avs",
    "textura_cargar texturas/cars/marcador.avs"
]

# Iluminacion interior de la cabina
posicion = punto(-6.5, 16.65, 3.30)

resultado.append(
    "luz " + formato((*posicion, 1.0))
)

resultado.extend(objetos)
resultado.append("fin")

SALIDA.write_text("\n".join(resultado) + "\n")

print("Normal del suelo:", normal)
print("Ecuacion del plano:", nx, ny, nz, d)
print("Escena generada:", SALIDA)
print("Primitivas adicionales:", len([
    l for l in objetos
    if l.startswith(
        ("poligono ", "cono ", "disco ", "cilindro ", "esfera ")
    )
]))