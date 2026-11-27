import bpy
import math
import struct

from pathlib import Path
from mathutils import Vector

# ==========================================
# CONFIGURACION
# ==========================================

ENTRADA = Path("modelos/mcqueen_calidad_media.glb")
SALIDA = Path("escenas_finales/mcqueen_importado.txt")
CARPETA_TEXTURAS = Path("texturas/mcqueen")

LONGITUD = 6.30

YAW = math.radians(32)
PITCH = math.radians(-18)

# Cambiar a True si el carro aparece de espaldas
INVERTIR_FRENTE = False

lineas = []
texturas = {}

CARPETA_TEXTURAS.mkdir(parents=True, exist_ok=True)
SALIDA.parent.mkdir(parents=True, exist_ok=True)


# ==========================================
# IMPORTAR MODELO
# ==========================================

bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

bpy.ops.import_scene.gltf(
    filepath=str(ENTRADA.resolve())
)

bpy.context.view_layer.update()

objetos = [
    obj for obj in bpy.context.scene.objects
    if obj.type == "MESH"
]


# ==========================================
# TRANSFORMACIONES
# ==========================================

def rotar(p):

    x, y, z = p

    x1 = x * math.cos(YAW) + z * math.sin(YAW)
    z1 = -x * math.sin(YAW) + z * math.cos(YAW)

    y2 = y * math.cos(PITCH) - z1 * math.sin(PITCH)
    z2 = y * math.sin(PITCH) + z1 * math.cos(PITCH)

    return x1, y2, z2


# Calcular dimensiones reales del modelo
vertices_globales = [
    obj.matrix_world @ vertice.co
    for obj in objetos
    for vertice in obj.data.vertices
]

minimo = Vector((
    min(v.x for v in vertices_globales),
    min(v.y for v in vertices_globales),
    min(v.z for v in vertices_globales)
))

maximo = Vector((
    max(v.x for v in vertices_globales),
    max(v.y for v in vertices_globales),
    max(v.z for v in vertices_globales)
))

dimensiones = maximo - minimo
centro = (maximo + minimo) / 2

# Detectar eje longitudinal del vehiculo
eje_longitudinal = (
    "X" if dimensiones.x > dimensiones.y else "Y"
)

escala = LONGITUD / max(
    dimensiones.x,
    dimensiones.y
)

print("Dimensiones:", dimensiones)
print("Eje longitudinal:", eje_longitudinal)
print("Escala:", escala)


def convertir_punto(p):

    x = (p.x - centro.x) * escala
    y = (p.y - centro.y) * escala
    z = (p.z - minimo.z) * escala

    # Blender utiliza Z como eje vertical.
    # Nuestro motor utiliza Y.

    if eje_longitudinal == "X":
        resultado = (y, z - 0.99, x)

    else:
        resultado = (x, z - 0.99, y)

    if INVERTIR_FRENTE:
        resultado = (
            -resultado[0],
            resultado[1],
            -resultado[2]
        )

    x, y, z = rotar(resultado)

    return x, y, z + 6


def punto_escena(p):

    x, y, z = rotar(p)

    return x, y, z + 6


def formato(valores):

    return " ".join(
        f"{v:.15f}" for v in valores
    )


# ==========================================
# CONVERTIR TEXTURAS A AVS
# ==========================================

def convertir_avs(imagen, ruta):

    ancho, alto = imagen.size

    pixeles = list(imagen.pixels)

    datos = bytearray()

    datos.extend(
        struct.pack(">II", ancho, alto)
    )

    # Blender almacena los pixeles desde abajo.
    # Nuestro AVS comienza arriba.

    for y in reversed(range(alto)):

        for x in range(ancho):

            i = (y * ancho + x) * 4

            r, g, b, a = pixeles[i:i + 4]

            def byte(v):
                return max(
                    0, min(255, round(v * 255))
                )

            datos.extend((
                byte(a),
                byte(r),
                byte(g),
                byte(b)
            ))

    ruta.write_bytes(datos)

    print("Textura generada:", ruta)


# ==========================================
# MATERIALES
# ==========================================

def obtener_material(material):

    color = (0.8, 0.8, 0.8)
    indice = 0

    if material is None:
        return color, indice

    if not material.use_nodes:
        return tuple(
            material.diffuse_color[:3]
        ), indice

    principled = next(
        (
            nodo
            for nodo in material.node_tree.nodes
            if nodo.type == "BSDF_PRINCIPLED"
        ),
        None
    )

    if principled is None:
        return color, indice

    base = principled.inputs.get("Base Color")

    if base is None:
        return color, indice

    color = tuple(base.default_value[:3])

    # Obtener textura conectada al color base
    if base.is_linked:

        nodo = base.links[0].from_node

        if nodo.type == "TEX_IMAGE" and nodo.image:

            imagen = nodo.image

            clave = imagen.name

            if clave not in texturas:

                indice = len(texturas) + 1

                ruta = CARPETA_TEXTURAS / (
                    f"textura_{indice}.avs"
                )

                convertir_avs(imagen, ruta)

                texturas[clave] = (
                    indice,
                    ruta
                )

            indice = texturas[clave][0]

    return color, indice


# Registrar materiales y texturas primero
materiales = {}

for obj in objetos:

    for material in obj.data.materials:

        if material is None:
            continue

        if material.name not in materiales:

            materiales[material.name] = (
                obtener_material(material)
            )


# ==========================================
# CONFIGURACION DE LA ESCENA
# ==========================================

lineas.extend([
    "# Cars - McQueen importado desde GLB",
    "# Modelo: Rookie Lightning McQueen",
    "# Autor: DisneyPixarCarsModels",
    "# Licencia: Creative Commons Attribution",
    "resolucion 650 380",
    "ojo 0 0 -14",
    "ventana -3.2 -1.87 3.2 1.87",
    "fondo 0.35 0.55 0.80",
    "ambiente 0.70",
    "antialiasing 1",
    "luz -6 9 -8 1.0",
    "luz 6 7 -4 0.30"
])


# ==========================================
# CARGAR TEXTURAS
# ==========================================

for indice, ruta in texturas.values():

    lineas.append(
        f"textura_cargar {ruta.as_posix()}"
    )


# ==========================================
# CONVERTIR TRIANGULOS
# ==========================================

total = 0
omitidos = 0

for obj in objetos:

    malla = obj.data

    malla.calc_loop_triangles()

    uv_layer = malla.uv_layers.active

    print(
        f"Procesando {obj.name}: "
        f"{len(malla.loop_triangles)} triangulos"
    )

    for triangulo in malla.loop_triangles:

        vertices = []

        for indice in triangulo.vertices:

            p = obj.matrix_world @ (
                malla.vertices[indice].co
            )

            vertices.append(
                convertir_punto(p)
            )

        A, B, C = map(Vector, vertices)

        # Descartar triangulos degenerados
        area_doble = (B - A).cross(C - A).length

        if area_doble < 1e-8:

            omitidos += 1
            continue

        color = (0.8, 0.8, 0.8)
        textura = 0

        indice_material = triangulo.material_index

        if indice_material < len(malla.materials):

            mat = malla.materials[indice_material]

            if mat is not None:

                color, textura = materiales.get(
                    mat.name,
                    ((0.8, 0.8, 0.8), 0)
                )

        coordenadas = []

        for vertice in vertices:
            coordenadas.extend(vertice)

        lineas.append(
            "poligono 3 " +
            formato(coordenadas) +
            " " +
            formato(color)
        )

        # Material sin reflexion recursiva
        # durante las pruebas de rendimiento
        lineas.append(
            "material 0.5 0.8 0.4 48 0 0"
        )

        if textura > 0 and uv_layer:

            lineas.append(
                f"textura {textura}"
            )

            coordenadas_uv = []

            for loop in triangulo.loops:

                uv = uv_layer.data[loop].uv

                coordenadas_uv.extend((
                    uv.x,
                    1.0 - uv.y
                ))

            lineas.append(
                "textura_uv " +
                formato(coordenadas_uv)
            )

        total += 1


# ==========================================
# PISTA
# ==========================================

vertices = [
    (-12, -1.0, -15),
    (12, -1.0, -15),
    (12, -1.0, 15),
    (-12, -1.0, 15)
]

datos = []

for vertice in vertices:
    datos.extend(punto_escena(vertice))

lineas.append(
    "poligono 4 " +
    formato(datos) +
    " 0.27 0.28 0.30"
)

lineas.append(
    "material 0.5 0.7 0.05 8 0 0"
)


# ==========================================
# ESFERAS AUXILIARES PARA VALIDACION
# ==========================================

for i in range(8):

    lineas.append(
        f"esfera {100 + i * 2} 0 5 0.1 0 0 0"
    )


# ==========================================
# GUARDAR ESCENA
# ==========================================

lineas.append("fin")

SALIDA.write_text(
    "\n".join(lineas) + "\n",
    encoding="utf-8"
)

print("\n========== RESULTADOS ==========")
print("Triangulos exportados:", total)
print("Triangulos omitidos:", omitidos)
print("Texturas exportadas:", len(texturas))
print("Escena generada:", SALIDA)