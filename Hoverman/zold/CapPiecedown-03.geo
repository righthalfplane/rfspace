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
rh=5.8/2;
hh=1.86;


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

Box(25) = {-d+10,-10,0, 2*l,20,h+r+4};

BooleanDifference(26)={Volume{24}; Delete;}{Volume{25}; Delete;};

w=1.1*(6.36/2);
y=1.1*(1.77+0.06);
t=1.86+0.5;

Box(31) = {-w,-y,0.0,    2*w,2*y,t};

Box(32) = {-w,-y,0.0,    2*w,2*y,t};

Box(33) = {-w,-y,0.0,    2*w,2*y,t};

Rotate{{0,0,1}, {0,0,0},6*Pi/9}{ Volume{32}; }

Rotate{{0,0,1}, {0,0,0},2*6*Pi/9}{ Volume{33}; }

BooleanUnion(34)={ Volume{31};  Delete; }{Volume{32,33};  Delete;};

//Rotate{{0,1,0}, {0,0,0},Pi/2}{ Volume{34}; }

v()=Translate {-d+5, -5.0, 0.0} { Duplicata { Volume{34}; } };

z()=Translate {-d-5,  5.0, 0.0} { Volume{34}; };

BooleanDifference(40)={Volume{26}; Delete;}{Volume{v(0),z(0)}; Delete;};

Box(41) = {-d-10,-10,h+r+2-0.2, 2*l,20,4};

BooleanDifference(42)={Volume{40}; Delete;}{Volume{41}; Delete;};


