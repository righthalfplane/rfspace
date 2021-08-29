// Gmsh project created on Fri May 28 11:18:05 2021
SetFactory("OpenCASCADE");


Box(1) = {-20,-4,0, 20,4,4};

Mesh.MeshSizeMin=0.2;
Mesh.MeshSizeMax=0.2;

Mesh.MeshSizeMin=0.4;
Mesh.MeshSizeMax=0.4;

Mesh.MeshSizeMin=0.6;
Mesh.MeshSizeMax=0.6;

Mesh.MeshSizeMin=0.5;
Mesh.MeshSizeMax=0.5;

Mesh.SubdivisionAlgorithm=2;

Mesh 3;

Cylinder(2) = {-5.0, -3.0, 0.0, 0, 0, 5, 2.0, 2*Pi};

Cylinder(3) = {-10.0, -3.0, 0.0, 0, 0, 5, 2.0, 2*Pi};

Cylinder(4) = {-15.0, -3.0, 0.0, 0, 0, 5, 2.0, 2*Pi};

Box(5) = {-12.0,-5,0, 4,2,4};

Box(6) = {-7.0,-5,0, 4,2,4};

Box(7) = {-17.0,-5,0, 4,2,4};

Cylinder(8) = {0.0, -3.0, 0.0, 0, 0, 5, 2.0, 2*Pi};

Cylinder(9) = {-20.0, -3.0, 0.0, 0, 0, 5, 2.0, 2*Pi};

Box(10) = {-2.0,-5,0, 4,2,4};

Box(11) = {-22.0,-5,0, 4,2,4};


f() = BooleanDifference { Volume{1}; Delete; }{ Volume{2,3,4,5,6,7,8,9,10,11}; Delete;};