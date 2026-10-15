"""Embed the engine's CJK font as a cooked, self-contained font face.

Use full editor -ExecutePythonScript (optionally -RenderOffscreen), not
-run=pythonscript: UE font import requires an initialized Slate application.
"""
import unreal

destination = "/Game/Foldlight/Fonts"
asset_path = destination + "/DroidSansFallback"
unreal.EditorAssetLibrary.make_directory(destination)
if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
    task = unreal.AssetImportTask()
    task.filename = unreal.Paths.engine_content_dir() + "Slate/Fonts/DroidSansFallback.ttf"
    task.destination_path = destination
    task.automated = True
    task.save = True
    task.factory = unreal.FontFileImportFactory()
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
face = unreal.EditorAssetLibrary.load_asset(asset_path)
if not isinstance(face, unreal.FontFace):
    raise RuntimeError("Missing Chinese font face")
face.set_editor_property("loading_policy", unreal.FontLoadingPolicy.INLINE)
unreal.EditorAssetLibrary.save_loaded_asset(face)
unreal.log("FOLDLIGHT_CHINESE_FONT_SUCCESS: embedded CJK face saved")
