import bpy
from pathlib import Path

ENTRADA = Path("modelos/rookie_lightning_mcqueen.glb")
SALIDA = Path("modelos/mcqueen_calidad_media.glb")

# Porcentajes de geometria que conservaremos
RATIO_CARROCERIA = 0.12
RATIO_LLANTAS = 0.20

# Limpiar escena de Blender
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)

# Importar modelo original
bpy.ops.import_scene.gltf(filepath=str(ENTRADA.resolve()))


def contar_triangulos(objeto):
    objeto.data.calc_loop_triangles()
    return len(objeto.data.loop_triangles)


total_antes = 0
total_despues = 0

for objeto in list(bpy.context.scene.objects):

    if objeto.type != "MESH":
        continue

    nombre = objeto.name.lower()
    antes = contar_triangulos(objeto)

    total_antes += antes

    # Seleccionar objeto
    bpy.ops.object.select_all(action="DESELECT")
    objeto.select_set(True)
    bpy.context.view_layer.objects.active = objeto

    # Conservar ojos y ventanas originales
    if "eye" in nombre or "glass" in nombre:
        ratio = 1.0

    elif "tire" in nombre or "wheel" in nombre:
        ratio = RATIO_LLANTAS

    else:
        ratio = RATIO_CARROCERIA

    if ratio < 1.0:

        modificador = objeto.modifiers.new(
            name="Simplificacion",
            type="DECIMATE"
        )

        modificador.decimate_type = "COLLAPSE"
        modificador.ratio = ratio

        bpy.ops.object.modifier_apply(
            modifier=modificador.name
        )

    despues = contar_triangulos(objeto)
    total_despues += despues

    print(
        f"{objeto.name}: "
        f"{antes} -> {despues} triangulos"
    )


# Exportar modelo simplificado
bpy.ops.export_scene.gltf(
    filepath=str(SALIDA.resolve()),
    export_format="GLB",
    export_texcoords=True,
    export_normals=True,
    export_materials="EXPORT"
)

print("\n========== RESULTADOS ==========")
print(f"Triangulos originales: {total_antes}")
print(f"Triangulos finales: {total_despues}")
print(f"Modelo generado: {SALIDA}")