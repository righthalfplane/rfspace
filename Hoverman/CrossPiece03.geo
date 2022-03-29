// Gmsh project created on Fri May 28 11:18:05 2021
SetFactory("OpenCASCADE");

Mesh.MeshSizeMin=1.0;
Mesh.MeshSizeMax=1.0;

//Mesh.SubdivisionAlgorithm=2;

h=2.0;
r=0.5*0.25*25.4+0.2;
d=82.86;
l=92.86;
r2=2.78/2+0.4;
rh=5.8/2+0.3;
hh=2.0;

Box(1)={-l,-10,0, 2*l,20,h};

Box(2) = {d-10,-10,h, 20,20,r+2};

Box(3) = {-d-10,-10,h, 20,20,r+2};

Cylinder(4) = {-d-20,0,h+r+2,  40,0,0, r, 2*Pi};

Rotate{{0,0,1}, {-d,0,0},Pi/4}{ Volume{4}; }

BooleanDifference(6)={Volume{3}; Delete;}{Volume{4}; Delete;};

Cylinder(5) = {d-20,0,h+r+2,  40,0,0, r, 2*Pi};

Rotate{{0,0,1}, {d,0,0},-Pi/4}{ Volume{5}; }

BooleanDifference(8)={Volume{2}; Delete;}{Volume{5}; Delete;};

//Box(10) = {-d+10,-10,0, 2*l,20,h+r+4};

//BooleanDifference(11)={Volume{1,8}; Delete;}{Volume{10}; Delete;};

Cylinder(12) = {0.0, 5.0, 0.0, 0, 0, 3, r2, 2*Pi};

Cylinder(14) = {0.0, -5.0, 0.0, 0, 0, 3, r2, 2*Pi};

BooleanDifference(15)={Volume{1,6,8}; Delete;}{Volume{12,14}; Delete;};

Cylinder(16) = {-d-5, 5.0, 0.0, 0, 0, h+r+4, r2, 2*Pi};

Cylinder(17) = {-d+5, -5.0, 0.0, 0, 0, h+r+4, r2, 2*Pi};

Cylinder(18) = {d+5, 5.0, 0.0, 0, 0, h+r+4, r2, 2*Pi};

Cylinder(19) = {d-5, -5.0, 0.0, 0, 0, h+r+4, r2, 2*Pi};

Cylinder(20) = {-d-5, 5.0, 0.0, 0, 0, hh, rh, 2*Pi};

Cylinder(21) = {-d+5, -5.0, 0.0, 0, 0, hh, rh, 2*Pi};

Cylinder(22) = {d+5, 5.0, 0.0, 0, 0, hh, rh, 2*Pi};

Cylinder(23) = {d-5, -5.0, 0.0, 0, 0, hh, rh, 2*Pi};

BooleanDifference(24)={Volume{15}; Delete;}{Volume{16,17,18,19,20,21,22,23}; Delete;};

//Box(21) = {-d+10,-10,0, 2*l,20,h+r+4};

//BooleanDifference(22)={Volume{20}; Delete;}{Volume{21}; Delete;};

