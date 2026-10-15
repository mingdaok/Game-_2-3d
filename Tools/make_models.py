"""Original Foldlight meshes. Deterministic, dependency-free OBJ authoring in cm.

All assets fit a 100 cm authoring envelope and are collision-free in game.
Source geometry is retained so the art can be edited without an external DCC.
"""
import math
from pathlib import Path

OUT = Path(__file__).parent / 'GeneratedModels'
OUT.mkdir(exist_ok=True)

class Mesh:
    def __init__(self):
        self.vertices, self.faces = [], []
    def face(self, points, reverse=False):
        start = len(self.vertices) + 1
        self.vertices.extend(points)
        for i in range(1, len(points)-1):
            tri = (start, start+i, start+i+1)
            self.faces.append(tri[::-1] if reverse else tri)
    def save(self, name):
        with (OUT / (name + '.obj')).open('w', encoding='ascii') as f:
            f.write('# Foldlight original procedural model; centimeters; Z up\n')
            for v in self.vertices: f.write('v %.6f %.6f %.6f\n' % v)
            f.write('s off\n')
            for tri in self.faces: f.write('f %d %d %d\n' % tri)
        print(name, len(self.faces), 'triangles')

def rings(mesh, rows, caps=True):
    n = len(rows[0])
    for a,b in zip(rows, rows[1:]):
        for i in range(n):
            j=(i+1)%n
            mesh.face([a[i],a[j],b[j],b[i]])
    if caps:
        mesh.face(rows[0], True)
        mesh.face(rows[-1])

def lathe(profile, segments=32, deform=None):
    mesh=Mesh(); rows=[]
    for k,(z,r) in enumerate(profile):
        row=[]
        for i in range(segments):
            a=i*math.tau/segments
            p=(r*math.cos(a),r*math.sin(a),z)
            row.append(deform(p,a,k) if deform else p)
        rows.append(row)
    rings(mesh,rows)
    return mesh

# Beveled slab, with clipped corners and a narrow chamfer on every edge.
m=Mesh(); rows=[]
for z,r,c in [(-50,46,3),(-46,50,5),(46,50,5),(50,46,3)]:
    rows.append([(r-c,r,z),(-r+c,r,z),(-r,r-c,z),(-r,-r+c,z),(-r+c,-r,z),(r-c,-r,z),(r,-r+c,z),(r,r-c,z)])
rings(m,rows);m.save('SM_Stone')

# Voussoir arch: individual wedges include narrow mortar gaps.
m=Mesh()
for i in range(17):
    a=i*math.pi/17+.008; b=(i+1)*math.pi/17-.008
    p=[(r*math.cos(t), y, r*math.sin(t)-25) for y in (-50,50) for r,t in [(38,a),(50,a),(50,b),(38,b)]]
    for ids in [(0,1,2,3),(7,6,5,4),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]:m.face([p[j] for j in ids])
m.save('SM_Arch')

# Ring lies in XY, centered on Z; can be rotated vertically for astrolabes.
m=Mesh()
for i in range(64):
    a=i*math.tau/64;b=(i+1)*math.tau/64
    p=[(r*math.cos(t),r*math.sin(t),z) for z in (-50,50) for r,t in [(44,a),(50,a),(50,b),(44,b)]]
    for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:m.face([p[j] for j in ids])
m.save('SM_Ring')
lathe([(-50,37),(-45,48),(-35,50),(-27,37),(-18,32),(26,29),(32,40),(42,40),(50,34)],24).save('SM_Pedestal')
lathe([(-50,0),(-31,34),(8,50),(29,27),(50,0)],7).save('SM_Crystal')
lathe([(-50,0),(-39,29),(-20,45),(0,40),(23,22),(40,8),(50,0)],24,
      lambda p,a,k:(p[0]+max(0,k-3)**2*1.4,p[1],p[2])).save('SM_Flame')

def wax(p,a,k):
    x,y,z=p
    if k in (3,4,5): z+=2.4*math.sin(5*a)+1.2*math.sin(9*a)
    return x,y,z
lathe([(-50,43),(-46,48),(29,48),(44,49),(48,44),(42,32),(36,20),(36,0)],48,wax).save('SM_Wax')

def cloth(p,a,k):
    x,y,z=p
    ripple=1+.055*math.cos(10*a)
    return x*ripple-8*(1-k/4), y*ripple, z+(3*math.cos(5*a) if k==0 else 0)
lathe([(-46,44),(-39,47),(-10,35),(24,23),(46,21)],48,cloth).save('SM_Cape')

# Twisting root silhouette, a single mesh replacing straight cylinder branches.
m=Mesh();rows=[]
for k in range(17):
    z=-50+k*100/16; x=13*math.sin(k*.58);y=7*math.sin(k*.91)
    r=8*(1-k/22)
    rows.append([(x+r*math.cos(i*math.tau/8),y+r*math.sin(i*math.tau/8),z) for i in range(8)])
rings(m,rows);m.save('SM_Root')

# Folded, double-sided leaf: raised midrib catches light without textures.
m=Mesh()
outline=[(0,0,-50),(-32,2,-18),(-42,1,15),(-18,0,37),(0,0,50),(18,0,37),(42,1,15),(32,2,-18)]
center=(0,10,4)
for i in range(len(outline)):
    tri=[outline[i],outline[(i+1)%len(outline)],center]
    m.face(tri);m.face(tri,True)
m.save('SM_Leaf')

# Individual grass blades in a compact tuft, authored upward from the floor.
m=Mesh()
for i in range(9):
    a=i*2.399963; r=6+(i%3)*8; h=58+(i*17)%40
    x,y=r*math.cos(a),r*math.sin(a)
    side=(-math.sin(a)*3.5,math.cos(a)*3.5,0)
    tri=[(x+side[0],y+side[1],0),(x-side[0],y-side[1],0),(x+22*math.cos(a),y+22*math.sin(a),h)]
    m.face(tri);m.face(tri,True)
m.save('SM_Grass')

# Hanging cloth with a folded surface and an irregular swallowtail hem.
m=Mesh()
for j in range(12):
    for i in range(8):
        def clothpoint(x,z):
            return (x,5*math.sin(x*.12+z*.035),z+(abs(x)*.28 if z<-45 else 0))
        quad=[clothpoint(-50+i*12.5,50-j*100/12),clothpoint(-50+(i+1)*12.5,50-j*100/12),clothpoint(-50+(i+1)*12.5,50-(j+1)*100/12),clothpoint(-50+i*12.5,50-(j+1)*100/12)]
        m.face(quad);m.face(quad,True)
m.save('SM_Banner')

# Chunky beveled gear; oriented like a ring in XY, with a hollow hub.
m=Mesh();n=96
for i in range(n):
    a=i*math.tau/n;b=(i+1)*math.tau/n
    ra=50 if i%8 in (2,3,4,5) else 42
    rb=50 if (i+1)%8 in (2,3,4,5) else 42
    p=[(r*math.cos(t),r*math.sin(t),z) for z in (-50,50) for r,t in [(25,a),(ra,a),(rb,b),(25,b)]]
    for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:m.face([p[k] for k in ids])
m.save('SM_Gear')

# Broken arch segment; missing stones give the arcade a less uniform silhouette.
m=Mesh()
for i in range(17):
    if i in (6,7,11):continue
    a=i*math.pi/17+.012;b=(i+1)*math.pi/17-.012
    p=[(r*math.cos(t),y,r*math.sin(t)-25) for y in (-50,50) for r,t in [(38,a),(50,a),(50,b),(38,b)]]
    for ids in [(0,1,2,3),(7,6,5,4),(0,4,5,1),(1,5,6,2),(2,6,7,3),(3,7,4,0)]:m.face([p[k] for k in ids])
m.save('SM_BrokenArch')

# Fine-line spell circle; architectural rings intentionally retain thick rims.
m=Mesh()
for i in range(64):
    a=i*math.tau/64;b=(i+1)*math.tau/64
    p=[(r*math.cos(t),r*math.sin(t),z) for z in (-50,50) for r,t in [(49,a),(50,a),(50,b),(49,b)]]
    for ids in [(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:m.face([p[k] for k in ids])
m.save('SM_Sigil')
