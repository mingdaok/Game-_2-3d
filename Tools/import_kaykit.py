"""Import selected author assets through UE's glTF Interchange pipeline."""
from pathlib import Path
import unreal, json
root=Path(__file__).parent/'ExternalAssets'
tools=unreal.AssetToolsHelpers.get_asset_tools()
target='/Game/Foldlight/External/KayKit'
def import_file(source,destination):
    task=unreal.AssetImportTask()
    task.filename=str(source);task.destination_path=destination
    task.automated=True;task.save=True;task.replace_existing=True
    tools.import_asset_tasks([task])
    return task.imported_object_paths

mage_path=target+'/Mage/Mage/SkeletalMeshes/Mage'
if not unreal.EditorAssetLibrary.does_asset_exist(mage_path):
    import_file(root/'Prepared/Mage.glb',target+'/Mage')
pack=next((root/'KayKitDungeon').glob('*/addons/kaykit_dungeon_remastered'))
names=['pillar_decorated','wall_arched','wall_broken','torch_mounted',
       'shelf_small_candles','table_small_decorated_A','barrel_large_decorated',
       'barrel_small_stack','candle_triple','rubble_large','wall_archedwindow_open','shelves']
for name in names:
    final=target+'/Environment/SM_KK_'+name
    if unreal.EditorAssetLibrary.does_asset_exist(final):continue
    source=next((pack/'Assets/gltf').glob(name+'.*.glb'))
    import_file(source,target+'/Imports/'+name)
    meshes=[unreal.load_asset(p) for p in unreal.EditorAssetLibrary.list_assets(target+'/Imports/'+name,recursive=True,include_folder=False)]
    meshes=[m for m in meshes if isinstance(m,unreal.StaticMesh)]
    assert len(meshes)==1,(name,len(meshes))
    assert unreal.EditorAssetLibrary.rename_asset(meshes[0].get_path_name(),final)

# Keep the author's palette textures, with a small ambient fill for dark puzzle rooms.
adventurers=next((root/'KayKitAdventurers').glob('*/addons/kaykit_character_pack_adventures'))
for label,source in [('Mage',adventurers/'Textures/mage_texture.png'),('Dungeon',pack/'Assets/texture/dungeon_texture.png')]:
    texture_path=target+'/Textures/'+source.stem
    if not unreal.EditorAssetLibrary.does_asset_exist(texture_path):import_file(source,target+'/Textures')
    texture=unreal.load_asset(texture_path)
    assert texture,texture_path
    material_path=target+'/M_KK_'+label
    material=unreal.load_asset(material_path) if unreal.EditorAssetLibrary.does_asset_exist(material_path) else tools.create_asset('M_KK_'+label,target,unreal.Material,unreal.MaterialFactoryNew())
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    material.set_editor_property('used_with_instanced_static_meshes',True)
    material.set_editor_property('used_with_skeletal_mesh',True)
    tex=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionTextureSample,-500,0)
    tex.texture=texture
    fill=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionMultiply,-200,140)
    fill.set_editor_property('const_b',.20 if label=='Mage' else .10)
    rough=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant,-200,300)
    rough.set_editor_property('r',.85)
    unreal.MaterialEditingLibrary.connect_material_property(tex,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_expressions(tex,'RGB',fill,'A')
    unreal.MaterialEditingLibrary.connect_material_property(fill,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    if label=='Mage':
        mesh=unreal.load_asset(mage_path)
        slots=mesh.get_editor_property('materials')
        for slot in slots:slot.material_interface=material
        mesh.set_editor_property('materials',slots)
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    else:
        for name in names:
            mesh=unreal.load_asset(target+'/Environment/SM_KK_'+name)
            for i in range(len(mesh.get_editor_property('static_materials'))):mesh.set_material(i,material)
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
records=[]
for path in unreal.EditorAssetLibrary.list_assets(target,recursive=True,include_folder=False):
    obj=unreal.load_asset(path)
    if obj is None:continue
    record={'path':path,'class':obj.get_class().get_name()}
    if isinstance(obj,unreal.SkeletalMesh):
        record['bounds']=str(obj.get_bounds())
        record['skeleton']=obj.get_editor_property('skeleton').get_path_name()
    if isinstance(obj,unreal.AnimSequence):record['length']=obj.get_play_length()
    if isinstance(obj,unreal.StaticMesh):record['bounds']=str(obj.get_bounds())
    unreal.log('KAYKIT_ASSET '+str(record));records.append(record)
(Path(__file__).parent/'ExternalAssets/Prepared/ue_assets.json').write_text(json.dumps(records,indent=2),encoding='utf8')
unreal.log('KAYKIT_IMPORT_COMPLETE')
