from pathlib import Path

BASE = Path("escenas_finales/cars_meta.txt")
SALIDA = Path("escenas_finales/piston_cup.txt")

lineas = BASE.read_text().splitlines()

lineas = [
    l for l in lineas
    if l.strip() != "fin"
]

num_texturas = sum(
    l.startswith("textura_cargar ")
    for l in lineas
)


def incorporar_carro(ruta, nombre):

    global num_texturas

    datos = Path(ruta).read_text().splitlines()

    texturas = [
        l for l in datos
        if l.startswith("textura_cargar ")
    ]

    desplazamiento = num_texturas

    lineas.extend(texturas)
    num_texturas += len(texturas)

    lineas.append(f"# VEHICULO {nombre}")

    conservar = False
    triangulos = 0

    for linea in datos:

        if linea.startswith("poligono "):

            conservar = linea.startswith("poligono 3 ")

            if conservar:
                lineas.append(linea)
                triangulos += 1

        elif linea.startswith("esfera "):

            conservar = False

        elif conservar:

            if linea.startswith("textura "):

                indice = int(linea.split()[1])

                if indice > 0:
                    indice += desplazamiento

                lineas.append(f"textura {indice}")

            elif linea.startswith("textura_uv "):

                lineas.append(linea)

            elif linea.startswith("material "):

                lineas.append(linea)

    print(
        nombre,
        "- Triangulos:", triangulos,
        "- Texturas:", len(texturas)
    )


incorporar_carro(
    "escenas_finales/king_importado.txt",
    "THE KING"
)

incorporar_carro(
    "escenas_finales/chick_importado.txt",
    "CHICK HICKS"
)

incorporar_carro(
    "escenas_finales/mate_importado.txt",
    "MATE"
)

incorporar_carro(
    "escenas_finales/doc_importado.txt",
    "DOC HUDSON"
)

# Configuracion para pruebas rapidas
for i, linea in enumerate(lineas):

    if linea.startswith("resolucion "):
        lineas[i] = "resolucion 650 380"

    elif linea.startswith("antialiasing "):
        lineas[i] = "antialiasing 0"

lineas.append("fin")

SALIDA.write_text(
    "\n".join(lineas) + "\n"
)

print("Texturas totales:", num_texturas)
print("Escena generada:", SALIDA)