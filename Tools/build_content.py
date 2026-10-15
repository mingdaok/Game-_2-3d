"""Editor-only asset authoring for the isolated Foldlight workspace.

Run with UnrealEditor-Cmd -run=pythonscript -script=.../build_content.py.
Writes only /Game/Foldlight assets in this project.
"""
import unreal
from pathlib import Path

ROOT = "/Game/Foldlight"
tools = unreal.AssetToolsHelpers.get_asset_tools()
unreal.EditorAssetLibrary.make_directory(ROOT + "/Art")
unreal.EditorAssetLibrary.make_directory(ROOT + "/Maps")
unreal.EditorAssetLibrary.make_directory(ROOT + "/Audio")
tasks = []
for wav in (Path(__file__).parent / "GeneratedAudio").glob("*.wav"):
    if unreal.EditorAssetLibrary.does_asset_exist(ROOT + "/Audio/" + wav.stem):
        continue
    task = unreal.AssetImportTask()
    task.filename = str(wav)
    task.destination_path = ROOT + "/Audio"
    task.automated = True
    task.save = True
    tasks.append(task)
if tasks:
    tools.import_asset_tasks(tasks)
for wav in (Path(__file__).parent / "GeneratedAudio").glob("*.wav"):
    sound = unreal.EditorAssetLibrary.load_asset(ROOT + "/Audio/" + wav.stem)
    if not sound:
        raise RuntimeError("Missing imported sound: " + wav.stem)
    sound.set_sound_asset_compression_type(unreal.SoundAssetCompressionType.PCM)
    unreal.EditorAssetLibrary.save_loaded_asset(sound)

material_path = ROOT + "/Art/M_Surface"
material = unreal.EditorAssetLibrary.load_asset(material_path) if unreal.EditorAssetLibrary.does_asset_exist(material_path) else None
if not material:
    material = tools.create_asset("M_Surface", ROOT + "/Art", unreal.Material, unreal.MaterialFactoryNew())
    color = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, -100)
    color.set_editor_property("parameter_name", "Tint")
    color.set_editor_property("default_value", unreal.LinearColor(0.2, 0.3, 0.3, 1))
    glow = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, 100)
    glow.set_editor_property("parameter_name", "Glow")
    glow.set_editor_property("default_value", unreal.LinearColor(0, 0, 0, 1))
    rough = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 300)
    rough.set_editor_property("r", 0.72)
    unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

map_path = ROOT + "/Maps/Trial"
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
    if not level.new_level(map_path):
        raise RuntimeError("Could not author the isolated trial map")
else:
    if not level.load_level(map_path):
        raise RuntimeError("Could not load the trial map")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not any(isinstance(a, unreal.PlayerStart) for a in actors.get_all_level_actors()):
    start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-1100, 0, 95), unreal.Rotator(0, 0, 0))
    start.set_actor_label("Foldlight_Start")
if not level.save_current_level():
    raise RuntimeError("Could not save the trial map")
unreal.log("FOLDLIGHT_CONTENT_SUCCESS: Trial map and surface material saved")
