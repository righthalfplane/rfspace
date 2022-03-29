// Gmsh project created on Fri May 28 11:18:05 2021
SetFactory("OpenCASCADE");

Mesh.MeshSizeMin=1.0;
Mesh.MeshSizeMax=1.0;

//Mesh.SubdivisionAlgorithm=2;

h=2.0;
r=0.5*0.25*25.4;
d=82.86;
l=92.86;

Box(1)={-l,-10,0, 2*l,20,h};

Box(2) = {d-10,-10,h, 20,20,r+2};

Box(3) = {-d-10,-10,h, 20,20,r+2};

Cylinder(4) = {-d-20,0,h+r+1.6,  40,0,0, r, 2*Pi};

Rotate{{0,0,1}, {-d,0,0},Pi/4}{ Volume{4}; }

BooleanDifference(6)={Volume{3}; Delete;}{Volume{4}; Delete;};

Cylinder(5) = {d-20,0,h+r+1.6,  40,0,0, r, 2*Pi};

Rotate{{0,0,1}, {d,0,0},-Pi/4}{ Volume{5}; }

BooleanDifference(8)={Volume{2}; Delete;}{Volume{5}; Delete;};

Box(10) = {-d+10,-10,0, 2*l,20,h+r+4};

BooleanDifference(11)={Volume{1,8}; Delete;}{Volume{10}; Delete;};
