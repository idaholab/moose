// This is a Gmsh input file. A mesh file with the .msh format can be created
// by running the command `gmsh -2 pipe.geo`

// This file is intended to describe an axisymmetric mesh for a circular pipe.
diameter = 1.0;
length = 40.0;

// Define the bounding vertices.
Point(1) = {0, 0, 0, 1};
Point(2) = {length, 0, 0, 1};
Point(3) = {0, diameter/2, 0, 1};
Point(4) = {length, diameter/2, 0, 1};

// Define the horizontal lines and their mesh size.
Line(1) = {1, 2};
Line(2) = {3, 4};
Transfinite Curve {1, 2} = 200;

// Define physical names for the horizontal lines.
Physical Curve("symmetry", 1) = {1};
Physical Curve("wall", 2) = {2};

// Define the vertical lines and mesh size. Use a geometric progression to
// better resolve the near-wall region.
Line(3) = {1, 3};
Line(4) = {2, 4};
Transfinite Curve {-3, -4} = 40 Using Progression 1.2;

// Define physical names for the vertical lines.
Physical Curve("inlet", 3) = {3};
Physical Curve("outlet", 4) = {4};

// Define the surface.
Curve Loop(1) = {1, 4, -2, -3};
Plane Surface(1) = {1};
Physical Surface("fluid", 5) = {1};

// Generate a quadrilateral mesh.
Transfinite Surface "*";
Recombine Surface "*";
