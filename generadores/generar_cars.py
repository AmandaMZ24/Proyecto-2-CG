import math
from pathlib import Path

lineas = []

# Rotacion del modelo para obtener una vista
# elevada de tres cuartos.
YAW = math.radians(32)
PITCH = math.radians(-18)


# ==========================================
# OPERACIONES GEOMETRICAS
# ==========================================

def rotar(p):
    x, y, z = p

    x1 = x * math.cos(YAW) + z * math.sin(YAW)
    z1 = -x * math.sin(YAW) + z * math.cos(YAW)

    y2 = y * math.cos(PITCH) - z1 * math.sin(PITCH)
    z2 = y * math.sin(PITCH) + z1 * math.cos(PITCH)

    return x1, y2, z2


def punto(p):
    x, y, z = rotar(p)
    return x, y, z + 6


def formato(valores):
    return " ".join(f"{v:.15f}" for v in valores)


def agregar(tipo, valores):
    lineas.append(tipo + " " + formato(valores))


def material(ka=0.3, kd=0.8, ks=0.5, kn=48,
             reflejo=0, transparencia=0):

    agregar("material", [
        ka, kd, ks, kn,
        reflejo, transparencia
    ])


# ==========================================
# PRIMITIVAS
# ==========================================

def esfera(centro, radio, color):
    agregar("esfera", [
        *punto(centro), radio, *color
    ])


def disco(centro, normal, radio, color):
    agregar("disco", [
        *punto(centro),
        *rotar(normal),
        radio,
        *color
    ])


def cilindro(ancla, eje, radio, d_min, d_max, color):
    agregar("cilindro", [
        *punto(ancla),
        *rotar(eje),
        radio,
        d_min,
        d_max,
        *color
    ])


def poligono(vertices, color):
    datos = [str(len(vertices))]

    for vertice in vertices:
        datos.append(formato(punto(vertice)))

    datos.append(formato(color))

    lineas.append("poligono " + " ".join(datos))


def elipsoide(centro, radios, color):
    """
    Genera una superficie cuadratica orientada
    mediante sus tres ejes principales.
    """

    c = punto(centro)

    ejes = [
        rotar((1, 0, 0)),
        rotar((0, 1, 0)),
        rotar((0, 0, 1))
    ]

    M = [[0.0] * 3 for _ in range(3)]

    for eje, radio in zip(ejes, radios):

        peso = 1.0 / (radio * radio)

        for i in range(3):
            for j in range(3):
                M[i][j] += peso * eje[i] * eje[j]

    mc = [
        sum(M[i][j] * c[j] for j in range(3))
        for i in range(3)
    ]

    A = M[0][0]
    B = M[1][1]
    C = M[2][2]

    D = 2 * M[0][1]
    E = 2 * M[0][2]
    F = 2 * M[1][2]

    G = -2 * mc[0]
    H = -2 * mc[1]
    I = -2 * mc[2]

    J = sum(c[i] * mc[i] for i in range(3)) - 1

    agregar("cuadratica", [
        A, B, C, D, E, F,
        G, H, I, J,
        *color
    ])


# ==========================================
# CONFIGURACION DE LA ESCENA
# ==========================================

lineas.extend([
    "# Cars - Rayo McQueen",
    "resolucion 650 380",
    "ojo 0 0 -14",
    "ventana -4.5 -2.63 4.5 2.63",
    "fondo 0.35 0.55 0.80",
    "ambiente 0.55",
    "antialiasing 0",
    "luz -6 8 -6 1.0",
    "luz 6 5 -3 0.65"
])


# ==========================================
# COLORES
# ==========================================

ROJO = (0.92, 0.025, 0.035)
ROJO_OSCURO = (0.45, 0.015, 0.02)
NEGRO = (0.025, 0.025, 0.03)
GRIS = (0.35, 0.35, 0.38)
PLATEADO = (0.75, 0.75, 0.78)
BLANCO = (0.97, 0.97, 0.97)
AZUL = (0.08, 0.45, 0.85)
AMARILLO = (1.0, 0.82, 0.02)
NARANJA = (1.0, 0.35, 0.01)
ROSADO = (0.85, 0.20, 0.32)


# ==========================================
# PISTA
# ==========================================

poligono([
    (-12, -0.98, -15),
    (12, -0.98, -15),
    (12, -0.98, 15),
    (-12, -0.98, 15)
], (0.22, 0.23, 0.25))

material(0.5, 0.7, 0.05, 8)


# ==========================================
# CARROCERIA
# ==========================================

# Cuerpo principal
elipsoide(
    (0, 0.08, 0),
    (1.65, 0.57, 3.15),
    ROJO
)

material(0.3, 0.8, 0.7, 80, 0.08)


# Capo delantero
elipsoide(
    (0, 0.30, -1.65),
    (1.52, 0.38, 1.60),
    ROJO
)

material(0.3, 0.8, 0.8, 90, 0.08)


# Parte trasera
elipsoide(
    (0, 0.30, 1.85),
    (1.55, 0.47, 1.30),
    ROJO
)

material(0.3, 0.8, 0.7, 80, 0.08)


# Parachoques delantero
elipsoide(
    (0, -0.18, -2.68),
    (1.38, 0.30, 0.55),
    ROJO
)


# ==========================================
# CABINA
# ==========================================

elipsoide(
    (0, 0.90, 0.40),
    (1.15, 0.67, 1.35),
    ROJO
)

material(0.3, 0.8, 0.7, 80, 0.08)


# Parabrisas blanco
poligono([
    (-1.05, 0.70, -1.42),
    (1.05, 0.70, -1.42),
    (0.90, 1.48, -0.90),
    (-0.90, 1.48, -0.90)
], BLANCO)

material(0.6, 0.6, 0.15, 32)


# Ventanas laterales trianguladas
for lado in [-1, 1]:

    A = (lado * 1.20, 0.75, -0.75)
    B = (lado * 1.20, 0.75, 1.20)
    C = (lado * 0.92, 1.43, 0.85)
    D = (lado * 0.92, 1.43, -0.65)

    poligono([A, B, C], (0.08, 0.12, 0.18))
    material(0.3, 0.5, 0.7, 80, 0.15)

    poligono([A, C, D], (0.08, 0.12, 0.18))
    material(0.3, 0.5, 0.7, 80, 0.15)


# ==========================================
# OJOS
# ==========================================

for lado in [-1, 1]:

    x = lado * 0.43

    # Iris azul
    elipsoide(
        (x, 1.08, -1.32),
        (0.27, 0.25, 0.13),
        AZUL
    )

    # Pupila
    elipsoide(
        (x + 0.025, 1.08, -1.43),
        (0.13, 0.15, 0.06),
        NEGRO
    )

    # Reflejo blanco
    esfera(
        (x + 0.07, 1.15, -1.48),
        0.045,
        BLANCO
    )


# ==========================================
# LLANTAS
# ==========================================

for lado in [-1, 1]:

    for z in [-1.85, 1.80]:

        centro = (
            lado * 1.28,
            -0.28,
            z
        )

        eje = (lado, 0, 0)

        # Caucho
        cilindro(
            centro,
            eje,
            0.70,
            -0.20,
            0.42,
            NEGRO
        )

        material(0.35, 0.7, 0.12, 12)

        # Cara exterior de la llanta
        exterior = (
            lado * 1.705,
            -0.28,
            z
        )

        disco(
            exterior,
            eje,
            0.70,
            NEGRO
        )

        # Aro rojo
        disco(
            (lado * 1.72, -0.28, z),
            eje,
            0.45,
            ROJO
        )

        material(0.3, 0.8, 0.8, 70, 0.12)

        # Centro metalico
        disco(
            (lado * 1.73, -0.28, z),
            eje,
            0.22,
            PLATEADO
        )

        material(0.25, 0.6, 0.9, 100, 0.35)

        # Tornillos
        for i in range(5):

            angulo = i * 2 * math.pi / 5

            esfera(
                (
                    lado * 1.74,
                    -0.28 + 0.32 * math.cos(angulo),
                    z + 0.32 * math.sin(angulo)
                ),
                0.055,
                PLATEADO
            )


# ==========================================
# FAROS
# ==========================================

for lado in [-1, 1]:

    elipsoide(
        (lado * 0.83, 0.34, -2.64),
        (0.32, 0.15, 0.28),
        BLANCO
    )

    material(0.6, 0.7, 0.8, 80)


# ==========================================
# BOCA Y LENGUA
# ==========================================

elipsoide(
    (0, -0.13, -3.02),
    (0.70, 0.21, 0.23),
    NEGRO
)

# Lengua del momento de la carrera
elipsoide(
    (0, -0.29, -3.42),
    (0.36, 0.11, 0.62),
    ROSADO
)

material(0.4, 0.8, 0.3, 24)

# Decoracion lateral de McQueen
for lado in [-1, 1]:

    x = lado * 1.66

    poligono([
        (x, 0.32, -1.10),
        (x, 0.32, 0.30),
        (x, 0.05, -0.20)
    ], AMARILLO)

    poligono([
        (x, 0.05, -0.20),
        (x, 0.20, 1.75),
        (x, -0.28, 0.35)
    ], AMARILLO)
    
# ==========================================
# ALERON TRASERO
# ==========================================

for lado in [-1, 1]:

    poligono([
        (lado * 1.0, 0.65, 2.10),
        (lado * 1.0, 0.65, 2.75),
        (lado * 1.0, 1.30, 2.75),
        (lado * 1.0, 1.30, 2.35)
    ], ROJO)


poligono([
    (-1.40, 1.30, 2.20),
    (1.40, 1.30, 2.20),
    (1.40, 1.40, 2.85),
    (-1.40, 1.40, 2.85)
], ROJO)

material(0.3, 0.8, 0.7, 64)


# ==========================================
# LINEA DE META
# ==========================================

for fila in range(3):

    for columna in range(20):

        x = -10 + columna
        z = -4.5 - fila * 0.5

        color = (
            BLANCO
            if (fila + columna) % 2 == 0
            else NEGRO
        )

        poligono([
            (x, -0.965, z),
            (x + 1, -0.965, z),
            (x + 1, -0.965, z - 0.5),
            (x, -0.965, z - 0.5)
        ], color)

        material(0.5, 0.8, 0.05, 8)


# ==========================================
# GUARDAR ESCENA
# ==========================================

lineas.append("fin")

ruta = Path("escenas_finales/mcqueen_prueba.txt")
ruta.parent.mkdir(parents=True, exist_ok=True)

ruta.write_text(
    "\n".join(lineas) + "\n",
    encoding="utf-8"
)

print(f"Escena generada: {ruta}")
print(f"Cantidad de instrucciones: {len(lineas)}")