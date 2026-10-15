"""Build the spell-only animated flame material; safe to rerun without importing other art."""
import unreal

path = '/Game/Foldlight/Art'
tools = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.MaterialEditingLibrary
mat = unreal.load_asset(path + '/M_FireFlow')
if not mat:
    mat = tools.create_asset('M_FireFlow', path, unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
mat.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
mat.set_editor_property('two_sided', True)
lib.delete_all_material_expressions(mat)

def node(cls, x, y, **props):
    n = lib.create_material_expression(mat, cls, x, y)
    for key, value in props.items():
        n.set_editor_property(key, value)
    return n

uv = node(unreal.MaterialExpressionTextureCoordinate, -800, 0)
inputs = [('UV', uv)]
for i, (name, default) in enumerate([('Age', 0), ('Seed', 0), ('Fade', 1), ('Ember', 0), ('Smoke', 0)]):
    inputs.append((name, node(unreal.MaterialExpressionScalarParameter, -800, 140+i*100,
                             parameter_name=name, default_value=default)))
shader = node(unreal.MaterialExpressionCustom, -350, 0,
              output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT4)
custom_inputs = []
for name, expression in inputs:
    entry = unreal.CustomInput()
    entry.set_editor_property('input_name', name)
    custom_inputs.append(entry)
shader.set_editor_property('inputs', custom_inputs)
shader.set_editor_property('code', r'''
float h = 1.0-UV.y;
float t = Age*3.7 + Seed*6.28318;
float x = (UV.x-.5)*2.0;
float bend = (sin(h*7-t)*.28 + sin(h*13-t*1.37+Seed)*.09)*h;
float q = abs(x-bend);
float flow = sin(h*19-t*3+sin(x*8+Seed)*1.7)*.5+.5;
float width = pow(saturate(1-h),.72)*(.63+.14*sin(h*11-t*2+Seed));
float body = 1-smoothstep(width-.055,width+.055,q);
float tip = 1-smoothstep(.81+.12*sin(t+Seed),1.0,h);
float base = smoothstep(0,.075,h);
float split = smoothstep(.15,.35,h)*(1-smoothstep(.55,.9,h));
float notch = smoothstep(.55,.9,flow)*smoothstep(.36,.68,q/max(width,.01))*split;
float a = saturate(body*tip*base*(1-notch*.9));
float core = (1-smoothstep(.15,.58,q/max(width,.01)))*(1-smoothstep(.25,.78,h));
float3 outer = lerp(float3(1,.045,.003),float3(1,.34,.009),saturate(1-q/max(width,.01)));
float3 color = lerp(outer,float3(1,.86,.27),core)*2.6;
float2 e = (UV-.5)*float2(2,2);
float spark = 1-smoothstep(.22,.95,length(e));
float smokeMask = pow(saturate(1-length(e+float2(sin(h*9-t)*.17,0))),2);
smokeMask *= .7+.3*sin(UV.x*17+h*13-t);
float3 resultColor = lerp(lerp(color,float3(3.8,1.65,.18),Ember),float3(.065,.052,.045),Smoke);
float resultAlpha = lerp(lerp(a*.88,spark*.9,Ember),smokeMask*.22,Smoke);
return float4(resultColor,resultAlpha*Fade);
''')
for name, expression in inputs:
    assert lib.connect_material_expressions(expression, '', shader, name), name
rgb = node(unreal.MaterialExpressionComponentMask, 0, 0, r=True, g=True, b=True, a=False)
alpha = node(unreal.MaterialExpressionComponentMask, 0, 180, r=False, g=False, b=False, a=True)
assert lib.connect_material_expressions(shader, '', rgb, '')
assert lib.connect_material_expressions(shader, '', alpha, '')
assert lib.connect_material_property(rgb, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
assert lib.connect_material_property(alpha, '', unreal.MaterialProperty.MP_OPACITY)
lib.recompile_material(mat)
assert unreal.EditorAssetLibrary.save_loaded_asset(mat), 'Could not save M_FireFlow'
unreal.log('FOLDLIGHT_FIRE_MATERIAL_SUCCESS')
