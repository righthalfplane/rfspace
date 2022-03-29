// Gmsh project created on Fri May 28 11:18:05 2021
SetFactory("OpenCASCADE");

Mesh.MeshSizeMin=1.0;
Mesh.MeshSizeMax=1.0;

//Mesh.SubdivisionAlgorithm=2;

h=2.0;
r=0.5*0.25*25.4+0.2;
d=62.86;
l=72.86;
r2=2.78/2+0.4;
rh=5.8/2+0.3;
hh=2.0;

Box(1)={-l,-10,0, 2*l,20,h};

Box(2) = {l-20,-10,0, 20,l,h};

Box(3) = {-d-10,-10,h, 20,20,r+2};

Box(40) = {l-20,l-30,h, 20,20,r+2};

Cylinder(4) = {-d-20,0,h+r+2,  40,0,0, r, 2*Pi};

Rotate{{0,0,1}, {-d,0,0},Pi/4}{ Volume{4}; }

BooleanDifference(6)={Volume{3}; Delete;}{Volume{4}; Delete;};

Cylinder(5) = {l-20,l-20,h+r+2,  40,0,0, r, 2*Pi};

BooleanDifference(8)={Volume{40}; Delete;}{Volume{5}; Delete;};

Cylinder(16) = {-d-5, 5.0, 0.0, 0, 0, h+r+4, r2, 2*Pi};

Cylinder(17) = {-d+5, -5.0, 0.0, 0, 0, h+r+4, r2, 2*Pi};

Cylinder(20) = {-d-5, 5.0, 0.0, 0, 0, hh, rh, 2*Pi};

Cylinder(21) = {-d+5, -5.0, 0.0, 0, 0, hh, rh, 2*Pi};


Cylinder(18) = {d+5, 6.0+l-20, 0.0, 0, 0, h+r+4, r2, 2*Pi};

//Cylinder(22) = {d+5, 6.0+l-20, 0.0, 0, 0, hh, rh, 2*Pi};

Cylinder(19) = {d-5, -6.0+l-20, 0.0, 0, 0, h+r+4, r2, 2*Pi};

//Cylinder(23) = {d-5, -6.0+l-20, 0.0, 0, 0, hh, rh, 2*Pi};

BooleanDifference(44)={Volume{1,6}; Delete;}{Volume{16,17,20,21}; Delete;};

BooleanDifference(45)={Volume{2,8}; Delete;}{Volume{18,19}; Delete;};

BooleanUnion(46)={Volume{44}; Delete;}{Volume{45}; Delete;};

Box(47) = {-d-10,-10,0, 2*l+20,l-20,h+r+2};

BooleanDifference(48)={Volume{46}; Delete;}{Volume{47}; Delete;};

Box(49) = {-d-10,-10,h+r+2-0.4, 2*l,l,4};

BooleanDifference(50)={Volume{48}; Delete;}{Volume{49}; Delete;};

