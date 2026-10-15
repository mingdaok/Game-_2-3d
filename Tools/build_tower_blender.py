"""Build an editable last-room model. Run with Blender --background --python."""
from pathlib import Path
import bpy, math, json, re
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'ArtSource' / 'Tower'
OUT.mkdir(parents=True, exist_ok=True)
source = (ROOT / 'Source/NEWMyProject/FDWorld.cpp').read_text(encoding='utf-8-sig')
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
for coll in list(bpy.data.collections):
    if coll.name != 'Collection': bpy.data.collections.remove(coll)
base = bpy.data.collections.get('Collection')
base.name = '00_Presentation'
def collection(name):
    c = bpy.data.collections.new(name); bpy.context.scene.collection.children.link(c); return c
geo = collection('01_Playable_Platforms')
details = collection('02_Stone_and_Brass_Details')
mechanisms = collection('03_Seals_and_Core')
guides = collection('04_Layout_Guides')
guides.hide_render = True
current = geo
def relocate(o, coll=None):
    for c in list(o.users_collection): c.objects.unlink(o)
    (coll or current).objects.link(o)
    return o
def mat(name, color, metallic=0, emission=0):
    m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
    p=next(n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
    p.inputs['Base Color'].default_value=(*color,1)
    p.inputs['Roughness'].default_value=.35 if metallic else .76
    p.inputs['Metallic'].default_value=metallic
    if emission:
        p.inputs['Emission Color'].default_value=(*color,1)
        p.inputs['Emission Strength'].default_value=emission
    return m
stone=mat('Basalt | blue grey',(.115,.17,.22))
deck=mat('Dressed stone | slate',(.27,.36,.43))
dark=mat('Recesses',(.025,.045,.06))
gold=mat('Aged brass',(.65,.37,.10),.75)
cyan=mat('Foldlight crystal',(.07,.7,.8),.3,2)
amber=mat('Directional seals',(.95,.39,.06),.4,1.3)
def finish(o,name,material):
    o.name=name; relocate(o); o.data.materials.append(material); return o
def box(name,loc,size,material=stone,bevel=.045):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    o=finish(bpy.context.object,name,material); o.dimensions=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        m=o.modifiers.new('Editable stone chamfer','BEVEL'); m.width=bevel; m.segments=2
        o.modifiers.new('Weighted corner normals','WEIGHTED_NORMAL')
    return o
def ring(name,loc,radius,minor=.035,material=gold,rotation=(0,0,0)):
    bpy.ops.mesh.primitive_torus_add(major_radius=radius,minor_radius=minor,major_segments=64,minor_segments=8,location=loc,rotation=rotation)
    return finish(bpy.context.object,name,material)
def crystal(name,loc,radius=.22,height=.65,material=cyan):
    # Faceted bipyramid, independently editable mesh.
    verts=[(0,0,-height*.5),(0,0,height*.5)]
    verts += [(radius*math.cos(i*math.tau/6),radius*math.sin(i*math.tau/6),-height*.12) for i in range(6)]
    faces=[]
    for i in range(6):
        a=2+i; b=2+(i+1)%6; faces += [(0,b,a),(1,a,b)]
    mesh=bpy.data.meshes.new(name); mesh.from_pydata(verts,[],faces); mesh.update()
    o=bpy.data.objects.new(name,mesh); current.objects.link(o); o.location=loc; o.data.materials.append(material); return o

# Read the eight elevated slabs directly from the live UE source.
pattern=r'Slab\(TEXT\("(r4_[^"]+)"\),FVector\(([-\d]+),([-\d]+),0\),([-\d]+),FVector2D\((\d+),(\d+)\)\)'
slabs=re.findall(pattern,source)
assert len(slabs)==8, 'Room source changed; review the slab parser.'
start=re.search(r'Platform\(4,TEXT\("r4_start"\),([^;]+)\);',source)
sx,sy,st,sw,sd=map(float,start[1].split(','))
pillar=re.search(r'Pillar\(4,TEXT\("r4_pillar"\),([^;]+)\);',source)
px,py,pt,tx=map(float,pillar[1].split(','))
layout=[('r4_start',sx,sy,st,sw,sd,st+180),('r4_pillar',px,py,pt,320,340,pt+180)]
layout += [(a,float(x),float(y),float(z),float(w),float(d),60) for a,x,y,z,w,d in slabs]
manifest=[]
for name,x,y,top,w,d,h in layout:
    x,y,top,w,d,h=[v/100 for v in (x,y,top,w,d,h)]
    current=geo
    o=box(name,(x,y,top-(h+.18)/2),(w,d,h-.18),stone)
    o['ue_object_id']=name; o['top_m']=top; o['ue_local_cm']=[x*100,y*100,(top-h/2)*100]
    manifest.append({'id':name,'position_m':list(o.location),'size_m':[w,d,h],'top_m':top})
    current=details
    children=[]
    children.append(box(name+'_deck',(x,y,top-.09),(w,d,.18),deck,.035))
    children.append(box(name+'_lower_course',(x,y,top-h+.10),(w*.96,d*.96,.20),dark,.03))
    for direction in [-1,1]:
        children.append(box(name+'_brass_X',(x,y+direction*(d/2+.004),top-.23),(w*.92,.025,.035),gold,.005))
        children.append(box(name+'_brass_Y',(x+direction*(w/2+.004),y,top-.23),(.025,d*.92,.035),gold,.005))
    tiles=max(2,math.ceil(w/2.2))
    for j in range(1,tiles): children.append(box(name+'_joint',(x-w/2+j*w/tiles,y,top+.002),(.02,d*.90,.004),dark,0))
    for offset in [-.3,0,.3]: children.append(box(name+'_joint',(x,y+d*offset,top+.004),(w*.92,.02,.005),dark,0))
    children.append(ring(name+'_inlay',(x,y,top+.018),.43,.012,gold))
    for child in children:
        bpy.context.view_layer.update()
        world=child.matrix_world.copy(); child.parent=o; child.matrix_world=world
    if name=='r4_pillar':
        o.keyframe_insert(data_path='location',frame=1)
        o.location.x=tx/100; o.keyframe_insert(data_path='location',frame=90)
        o.location.x=x
        current=guides
        target=box('Pillar_destination',(tx/100,y,top-h/2),(w,d,h),cyan,0)
        target.display_type='WIRE'; target.hide_render=True
        current=details
        for dy in [-1.7,1.7]: box('Stone_pillar_rail',((px+tx)/200,y+dy,-1.66),(abs(tx-px)/100+3.2,.08,.08),gold,.015)

# Cardinal seals use the same offset and view basis as FDProjection.h.
lookup={r[0]:r for r in layout}
current=mechanisms
for name,right,title in [('r4_south',(1,0),'South'),('r4_west',(0,1),'West'),('r4_north',(-1,0),'North'),('r4_goal',(0,-1),'East')]:
    row=lookup[name]; x,y,top=row[1]/100,row[2]/100,row[3]/100
    rx,ry=right; ax,ay=x+rx*2.3,y+ry*2.3
    # Source receiver centre = slab centre + 1.50 m.
    z=top+1.2
    box(title+'_seal_pedestal',(ax,ay,z-.18),(1,1,.38),stone)
    ring(title+'_seal_ring',(ax,ay,z+.07),.375,.045,gold)
    crystal(title+'_seal',(ax,ay,z+.30),.215,.65,amber)
    for i in range(4):
        angle=i*math.pi/2
        crystal(title+'_satellite',(ax+math.cos(angle)*.33,ay+math.sin(angle)*.33,z+.20),.07,.48,amber)
        pos=(x+rx*(i*.35-.55),y+ry*(i*.35-.55),top+.02)
        size=(.07,1-i*.22,.03) if rx else (1-i*.22,.07,.03)
        box(title+'_direction_mark',pos,size,gold,.004)

# Terminal star-ring, dimensions from FDObject's Exit appearance.
current=mechanisms
x,y,z=.0+lookup['r4_goal'][1]/100,lookup['r4_goal'][2]/100,lookup['r4_goal'][3]/100+.08
box('Core_base',(x,y,z-.08),(2.05,2.05,.26),stone)
for side in [-1,1]:
    box('Core_obelisk',(x+side*1.04,y,z+.75),(.42,.42,1.5),deck)
    crystal('Core_obelisk_tip',(x+side*1.04,y,z+1.64),.125,.5)
ring('Core_outer_astrolabe',(x,y-.10,z+1.65),1.4,.065,gold,(math.pi/2,0,0))
ring('Core_inner_light',(x,y-.10,z+1.65),1.205,.022,cyan,(math.pi/2,0,0))
for i in range(12):
    a=i*math.tau/12
    o=box('Core_ring_tick',(x+math.cos(a)*1.31,y-.20,z+1.65+math.sin(a)*1.31),(.10,.08,.18),gold,.012)
    o.rotation_euler.y=math.pi/2-a
crystal('Core_crown',(x,y-.08,z+2.64),.12,.42,amber)

scene=bpy.context.scene
scene.unit_settings.system='METRIC'; scene.unit_settings.scale_length=1
scene.frame_end=90; scene.frame_set(1)
scene['description']='Foldlight room 5; platform layout parsed from FDWorld.cpp. UE local centimetres / 100. Decoration reconstructed, not an asset extraction.'
scene['coordinates']='X,Y,Z match UE local numeric axes; metres. UE handedness must be considered when reimporting.'
current=base
def camera(name,pos,target,scale):
    bpy.ops.object.camera_add(location=pos); o=relocate(bpy.context.object,base); o.name=name
    o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
    o.data.type='ORTHO'; o.data.ortho_scale=scale; o.data.lens=45; o.data.clip_end=500
    return o
target=(-7.0,-5.0,2.5)
cameras={
 'Overview':camera('Camera_Overview',(35,43,39),target,47),
 'Front':camera('Camera_Front_South',(-7,60,3),(-7,-5,3),42),
 'Side':camera('Camera_Side_East',(60,-5,3),(-7,-5,3),42),
 'Top':camera('Camera_Top',(-7,-5,70),(-7,-5,0),42)}
cameras['Top'].rotation_euler=(0,0,0)
def area(name,pos,energy,size,color):
    bpy.ops.object.light_add(type='AREA',location=pos); o=relocate(bpy.context.object,base); o.name=name
    o.data.energy=energy; o.data.shape='DISK'; o.data.size=size; o.data.color=color
    o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler()
area('Key_softbox',(0,10,30),14000,18,(.72,.86,1))
area('Warm_rim',(-20,-18,22),18000,14,(1,.69,.38))
area('Front_fill',(20,-25,12),10000,16,(.46,.77,1))
scene.world.use_nodes=True
next(n for n in scene.world.node_tree.nodes if n.type == 'BACKGROUND').inputs[0].default_value=(.055,.08,.12,1)
next(n for n in scene.world.node_tree.nodes if n.type == 'BACKGROUND').inputs[1].default_value=.5
scene.render.engine='CYCLES'; scene.cycles.samples=32
scene.cycles.use_denoising=True
scene.render.resolution_x=1600; scene.render.resolution_y=1200; scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG'
scene.view_settings.view_transform='AgX'
scene.camera=cameras['Overview']
for screen in bpy.data.screens:
    for a in screen.areas:
        if a.type=='VIEW_3D':
            a.spaces.active.region_3d.view_distance=46
            a.spaces.active.region_3d.view_location=target
            a.spaces.active.region_3d.view_rotation=scene.camera.rotation_euler.to_quaternion()
            a.spaces.active.clip_end=500
            a.spaces.active.shading.color_type='MATERIAL'
bpy.ops.object.select_all(action='DESELECT')
for c in [geo,details,mechanisms]:
    for o in c.objects: o.select_set(True)
bpy.context.view_layer.objects.active=geo.objects[0]
# Static first-frame GLB; source blend retains editable modifiers and pillar motion.
bpy.ops.export_scene.gltf(filepath=str(OUT/'Foldlight_Tower.glb'),export_format='GLB',use_selection=True,export_animations=False,export_apply=True)
bpy.ops.object.select_all(action='DESELECT')
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'Foldlight_Tower.blend'))
(OUT/'layout_manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
for name,cam in cameras.items():
    scene.camera=cam
    scene.render.filepath=str(OUT/(name+'.png'))
    bpy.ops.render.render(write_still=True)
print('TOWER_BUILD_COMPLETE',OUT)

