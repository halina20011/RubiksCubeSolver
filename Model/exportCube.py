import bpy
import math

selected = bpy.context.collection;
#print(selected);

def comp(a, b):
    if(a < b):
        return 1;
    return -1;

def join(arr, l, m, end):
    p1 = l;
    p2 = m;
    temp = [];
    while(p1 < m and p2 < end):
        if(arr[p1].comp(arr[p2]) == 1):
            temp.append(arr[p1]);
            p1 += 1;
        else:
            temp.append(arr[p2]);
            p2 += 1;
            
    while(p1 < m):
        temp.append(arr[p1]);
        p1+=1;
        
    while(p2 < end):
        temp.append(arr[p2]);
        p2+=1;
        
    j = 0;
    # print(temp);
    for i in range(l, end):
        arr[i] = temp[j];
        j+=1;

def sort(arr, l, r):
    if(r == l):
        return;
    
    m = math.floor((r - l) / 2) + l;
    sort(arr, l, m);
    sort(arr, m + 1, r);
    join(arr, l, m + 1, r + 1);

def Sort(arr):
    sort(arr, 0, len(arr) - 1);

# arr = [9,8,5,4,2,7,7,3,9,0,1,2,7,3,5,24,0,234,9732,31,1,0];
# Sort(arr, comp);
# print(arr);

class Vec3:
    def __init__(self, vec):
        self.x = vec.x;
        self.y = vec.y;
        self.z = vec.z;

    def __str__(self):
        return "{:.5f} {:.5f} {:.5f}".format(self.x, self.y, self.y);

class Mesh:
    def __init__(self, mesh):
        self.name = mesh.name;
        self.mesh = mesh;
        self.location = self.getLocation();
        
    def getLocation(self):
        obj.select_set(True);
        bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='MEDIAN');
        
        location = Vec3(self.mesh.location);

        bpy.ops.object.origin_set(type='ORIGIN_CURSOR', center='MEDIAN');

        obj.select_set(False);

        return location;

    def comp(self, y):
        a = self.location;
        b = y.location;
        # print(a, b);
    
        if(a.z < b.z):
            return -1;
        if(a.y < b.y):
            return -1;
        return 1;

cubes = [];
planes = [];
for obj in selected.objects:
    if(obj.type == "MESH"):
        name = obj.name;
        if("Cube" in name or "cube" in name):
            cubes.append(Mesh(obj));
        else:
            planes.append(obj);

Sort(cubes);
#print(cubes);

for c in cubes:
    print("{} {:.4f}".format(c.name, c.location.z));
