import bpy
import bmesh
import struct
import os

class Vec3:
    def __init__(self, arr):
        self.x = arr[0];
        self.y = arr[1];
        self.z = arr[2];
        
    def __str__(self):
        return "{:.4f} {:.4f} {:.4f}".format(self.x, self.y, self.z);

class Mesh:
    def __init__(self, mesh, f):
        self.mesh = mesh;
        self.color = self.matrial();
        self.write(f);

    def matrial(self):
        matrial = self.mesh.active_material;
        if(matrial and matrial.use_nodes and matrial.node_tree.nodes.get("Principled BSDF")):
            pN = matrial.node_tree.nodes["Principled BSDF"]
            if(pN):
                return Vec3(pN.inputs["Base Color"].default_value);
            
        return Vec3([0, 0, 0]);
    
    def write(self, f):
        graph = bpy.context.evaluated_depsgraph_get();
        evalObj = self.mesh.evaluated_get(graph);
        meshData = evalObj.data;
        
        bm = bmesh.new();
        bm.from_mesh(meshData);
        
        bmesh.ops.triangulate(bm, faces=bm.faces[:]);
        
        data = [];
        x = self.color.x / 1 * 255;
        y = self.color.y / 1 * 255;
        z = self.color.z / 1 * 255;
        tM = self.mesh.matrix_world;
        for face in bm.faces:
            for v in face.verts:
                co = v.co @ tM;
                data.append(co.x);
                data.append(co.y);
                data.append(co.z);
                
                data.append(self.color.x);
                data.append(self.color.y);
                data.append(self.color.z);
            
        print("{} => {} {:.4f} {:.4f} {:.4f}".format(self.mesh.name, len(data), x, y, z));
        f.write(struct.pack("i", len(data)));
        f.write(struct.pack(f"{len(data)}f", *data));
        
        del(evalObj);
        bm.free();

def save(f, fStruct):
    selected = bpy.context.collection;

    for coll in selected.children_recursive:
        print("========= {} ==========".format(coll.name));
        fStruct.write(struct.pack("i", len(coll.objects)));
        for mesh in coll.objects:
            m = Mesh(mesh, f);
            
    print("saved {}".format(path));

folder = bpy.path.abspath("//");
path = os.path.join(folder, "model.bin");
structPath = os.path.join(folder, "struct.bin");
f = open(path, "wb+");
fStruct = open(structPath, "wb+");

save(f, fStruct);

#obj = bpy.context.active_object
#m = Mesh(obj, f);

f.close();