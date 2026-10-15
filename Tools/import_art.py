"""Import original meshes and author a world-space stone grain material."""
import unreal
from pathlib import Path

root='/Game/Foldlight/Art'
tools=unreal.AssetToolsHelpers.get_asset_tools()
for path in sorted((Path(__file__).parent/'GeneratedModels').glob('*.obj')):
    task=unreal.AssetImportTask()
    task.filename=str(path)
    task.destination_path=root+'/Models'
    task.destination_name=path.stem
    task.automated=True
    task.replace_existing=True
    task.save=True
    task.factory=unreal.FbxFactory()
    options=unreal.FbxImportUI()
    options.import_mesh=True
    options.import_materials=False
    options.import_textures=False
    options.import_as_skeletal=False
    options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True
    options.static_mesh_import_data.auto_generate_collision=False
    options.static_mesh_import_data.generate_lightmap_u_vs=False
    options.static_mesh_import_data.convert_scene=False
    task.options=options
    tools.import_asset_tasks([task])
    mesh=unreal.load_asset(root+'/Models/'+path.stem)
    if not mesh: raise RuntimeError('Model import failed: '+path.stem)
    bounds=mesh.get_bounding_box()
    unreal.log('FOLDLIGHT_MODEL '+path.stem+' bounds='+str(bounds))

mat=unreal.load_asset(root+'/M_Surface')
mat.set_editor_property('used_with_instanced_static_meshes',True)
lib=unreal.MaterialEditingLibrary
lib.delete_all_material_expressions(mat)
def node(cls,x,y,**props):
    n=lib.create_material_expression(mat,cls,x,y)
    for k,v in props.items():n.set_editor_property(k,v)
    return n
def wire(a,out,b,slot):lib.connect_material_expressions(a,out,b,slot)
tint=node(unreal.MaterialExpressionVectorParameter,-700,-200,parameter_name='Tint',default_value=unreal.LinearColor(.2,.3,.3,1))
glow=node(unreal.MaterialExpressionVectorParameter,-300,300,parameter_name='Glow')
grain=node(unreal.MaterialExpressionScalarParameter,-700,100,parameter_name='Grain',default_value=0.0)
rough=node(unreal.MaterialExpressionScalarParameter,-300,450,parameter_name='Roughness',default_value=.78)
metal=node(unreal.MaterialExpressionScalarParameter,-300,550,parameter_name='Metallic',default_value=0.0)
noise=node(unreal.MaterialExpressionNoise,-900,0,scale=.045,quality=1,levels=2,output_min=.82,output_max=1.0)
lerp=node(unreal.MaterialExpressionLinearInterpolate,-450,0,const_a=1.0)
wire(noise,'',lerp,'B');wire(grain,'',lerp,'Alpha')
mul=node(unreal.MaterialExpressionMultiply,-200,-100)
wire(tint,'',mul,'A');wire(lerp,'',mul,'B')
for n,prop in [(mul,unreal.MaterialProperty.MP_BASE_COLOR),(glow,unreal.MaterialProperty.MP_EMISSIVE_COLOR),(rough,unreal.MaterialProperty.MP_ROUGHNESS),(metal,unreal.MaterialProperty.MP_METALLIC)]:
    lib.connect_material_property(n,'',prop)
lib.recompile_material(mat)
unreal.EditorAssetLibrary.save_loaded_asset(mat)

# A separate translucent, unlit spell surface prevents dark metallic flame beads
# and avoids the opaque bloom mass of the previous large stock fire emitter.
spell=unreal.load_asset(root+'/M_Spell')
if not spell:spell=tools.create_asset('M_Spell',root,unreal.Material,unreal.MaterialFactoryNew())
spell.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
spell.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
lib.delete_all_material_expressions(spell)
emission=lib.create_material_expression(spell,unreal.MaterialExpressionVectorParameter,-200,0)
emission.set_editor_property('parameter_name','Glow')
emission.set_editor_property('default_value',unreal.LinearColor(1,.2,.025,1))
opacity=lib.create_material_expression(spell,unreal.MaterialExpressionConstant,-200,150)
opacity.set_editor_property('r',.62)
lib.connect_material_property(emission,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
lib.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
lib.recompile_material(spell)
unreal.EditorAssetLibrary.save_loaded_asset(spell)
unreal.log('FOLDLIGHT_ART_SUCCESS')
