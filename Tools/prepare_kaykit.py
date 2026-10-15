"""Prepare CC0 KayKit assets for UE without requiring an external DCC.

Keeps the original downloads intact. Retains eight gameplay clips and binds
the author's hat/cape/staff rigid geometry to their original attachment bones.
"""
import json, struct, hashlib, shutil
from pathlib import Path
import numpy as np

ROOT=Path(__file__).parent/'ExternalAssets'
OUT=ROOT/'Prepared'
OUT.mkdir(exist_ok=True)
source=next((ROOT/'KayKitAdventurers').rglob('Mage.glb'))
raw=source.read_bytes()
length=struct.unpack_from('<I',raw,12)[0]
data=json.loads(raw[20:20+length])
binary=bytearray(raw[28+length:])
nodes=data['nodes']
parents={c:i for i,n in enumerate(nodes) for c in n.get('children',[])}

def matrix(node):
    if 'matrix' in node:return np.array(node['matrix']).reshape(4,4).T
    x,y,z,w=node.get('rotation',[0,0,0,1])
    r=np.array([[1-2*(y*y+z*z),2*(x*y-z*w),2*(x*z+y*w)],
                [2*(x*y+z*w),1-2*(x*x+z*z),2*(y*z-x*w)],
                [2*(x*z-y*w),2*(y*z+x*w),1-2*(x*x+y*y)]])
    m=np.eye(4);m[:3,:3]=r@np.diag(node.get('scale',[1,1,1]));m[:3,3]=node.get('translation',[0,0,0])
    return m
def world(i):
    return (world(parents[i]) if i in parents else np.eye(4))@matrix(nodes[i])
def read_accessor(index):
    a=data['accessors'][index]; v=data['bufferViews'][a['bufferView']]
    dtype={5126:'<f4',5123:'<u2',5125:'<u4',5121:'u1'}[a['componentType']]
    width={'SCALAR':1,'VEC2':2,'VEC3':3,'VEC4':4,'MAT4':16}[a['type']]
    start=v.get('byteOffset',0)+a.get('byteOffset',0)
    return np.ndarray((a['count'],width),dtype=dtype,buffer=binary,offset=start,
                      strides=(v.get('byteStride',np.dtype(dtype).itemsize*width),np.dtype(dtype).itemsize)).copy()
def append(array,kind,component=5126):
    while len(binary)%4:binary.append(0)
    offset=len(binary);blob=array.astype('<f4' if component==5126 else '<u2').tobytes();binary.extend(blob)
    vi=len(data['bufferViews']);data['bufferViews'].append({'buffer':0,'byteOffset':offset,'byteLength':len(blob)})
    ai=len(data['accessors']);a={'bufferView':vi,'componentType':component,'count':len(array),'type':kind}
    if kind=='VEC3':a.update(min=array.min(axis=0).tolist(),max=array.max(axis=0).tolist())
    data['accessors'].append(a);return ai

rig=53
for node_id,bone_id in [(14,15),(16,17),(8,9)]:
    n=nodes[node_id];transform=np.linalg.inv(world(rig))@world(node_id)
    joint=data['skins'][0]['joints'].index(bone_id)
    for primitive in data['meshes'][n['mesh']]['primitives']:
        at=primitive['attributes'];positions=read_accessor(at['POSITION'])
        positions=(transform@np.c_[positions,np.ones(len(positions))].T).T[:,:3]
        at['POSITION']=append(positions,'VEC3')
        if 'NORMAL' in at:
            normals=(np.linalg.inv(transform[:3,:3]).T@read_accessor(at['NORMAL']).T).T
            normals/=np.maximum(np.linalg.norm(normals,axis=1)[:,None],1e-8)
            at['NORMAL']=append(normals,'VEC3')
        at.pop('TANGENT',None)
        joints=np.zeros((len(positions),4));joints[:,0]=joint
        weights=np.zeros((len(positions),4));weights[:,0]=1
        at['JOINTS_0']=append(joints,'VEC4',5123);at['WEIGHTS_0']=append(weights,'VEC4')
    nodes[parents[node_id]]['children'].remove(node_id)
    nodes[rig]['children'].append(node_id)
    for key in ('translation','rotation','scale','matrix'):n.pop(key,None)
    n['skin']=0
for i in (0,1,7):nodes[i].pop('mesh',None)
keep={'Idle','Walking_A','Running_A','Jump_Start','Jump_Idle','Jump_Land','Spellcast_Shoot','Interact'}
data['animations']=[a for a in data['animations'] if a['name'] in keep]
for a in data['animations']:
    a['channels']=[c for c in a['channels'] if not(c['target']['node']==46 and c['target']['path']=='translation')]
data['buffers'][0]['byteLength']=len(binary)
blob=json.dumps(data,separators=(',',':')).encode('utf8')
blob+=b' '*((-len(blob))%4);binary+=b'\0'*((-len(binary))%4)
output=struct.pack('<III',0x46546c67,2,28+len(blob)+len(binary))+struct.pack('<II',len(blob),0x4e4f534a)+blob+struct.pack('<II',len(binary),0x004e4942)+binary
(OUT/'Mage.glb').write_bytes(output)
print('Prepared mage:',len(data['animations']),'clips; hat/cape/staff bound to author bones')
manifest=[]
for name in ('KayKitAdventurers','KayKitDungeon'):
    archive=ROOT/(name+'.zip')
    manifest.append({'pack':name,'sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'bytes':archive.stat().st_size})
    license_file=next((ROOT/name).rglob('LICENSE.txt'))
    shutil.copyfile(license_file,OUT/(name+'-CC0.txt'))
(OUT/'downloads.json').write_text(json.dumps(manifest,indent=2),encoding='utf8')
