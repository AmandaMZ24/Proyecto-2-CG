import random
import struct
from pathlib import Path

random.seed(95)

ANCHO = 2048
ALTO = 48

SALIDA = Path("texturas/cars/publico.avs")

pixeles = bytearray(
    [255, 35, 38, 45] * ANCHO * ALTO
)


def pixel(x, y, color):

    if 0 <= x < ANCHO and 0 <= y < ALTO:

        i = (y * ANCHO + x) * 4

        pixeles[i:i+4] = bytes(
            (255, *color)
        )


def rectangulo(x, y, w, h, color):

    for py in range(y, y + h):
        for px in range(x, x + w):
            pixel(px, py, color)


def elipse(cx, cy, rx, ry, color):

    for y in range(cy - ry, cy + ry + 1):
        for x in range(cx - rx, cx + rx + 1):

            if (
                ((x - cx) / rx)**2
                + ((y - cy) / ry)**2
                <= 1
            ):
                pixel(x, y, color)


COLORES = [
    (210, 45, 40),
    (35, 115, 200),
    (220, 180, 35),
    (45, 160, 80),
    (175, 65, 150),
    (200, 200, 205),
    (230, 110, 35),
    (45, 165, 180),
    (110, 85, 180)
]

for fila in range(1):

    for columna in range(85):

        # Pasillos entre secciones
        if columna % 22 in (0, 1):
            continue

        x = 12 + columna * 24
        y = 20 + fila * 48

        x += random.randint(-3, 3)
        y += random.randint(-3, 3)

        color = random.choice(COLORES)

        oscuro = tuple(
            int(c * 0.65)
            for c in color
        )

        # Sombra inferior
        elipse(x, y + 10, 10, 3, (15, 15, 20))

        # Llantas
        rectangulo(x - 9, y + 3, 4, 9, (20, 20, 20))
        rectangulo(x + 5, y + 3, 4, 9, (20, 20, 20))

        # Carroceria
        elipse(x, y + 3, 10, 7, color)

        # Techo
        elipse(x, y - 5, 7, 6, oscuro)

        # Parabrisas
        rectangulo(
            x - 5, y - 6, 10, 5,
            (175, 200, 215)
        )

        # Faros
        rectangulo(
            x - 7, y + 5, 3, 2,
            (245, 235, 185)
        )

        rectangulo(
            x + 4, y + 5, 3, 2,
            (245, 235, 185)
        )


SALIDA.parent.mkdir(
    parents=True,
    exist_ok=True
)

with SALIDA.open("wb") as archivo:

    archivo.write(
        struct.pack(">II", ANCHO, ALTO)
    )

    archivo.write(pixeles)

print("Textura generada:", SALIDA)