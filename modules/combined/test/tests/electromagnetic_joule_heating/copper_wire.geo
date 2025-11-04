// Copper wire mesh for microwave heating in a copper wire
// This file should be used with the Gmsh mesh generator (https://gmsh.info/)
// Wire length = 4.5 cm
// Wire diameter = 0.5 cm
// Entry location (lower left coordinate) = (0.0, 0.0)

// Declare scaling factor and second order meshing
sf = 0.002;
Mesh.ElementOrder = 2;
Mesh.SecondOrderLinear = 0;


// Define corners and important points in the domain (with scaling factors)
Point(1) = {-0, 0, 0, sf};
Point(2) = {0, 0.005, 0, sf};
Point(3) = {0.045, 0.005, 0, sf};
Point(4) = {0.045, 0, 0, sf};

// Connect the outer points
Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};

// Define loop for wire
Line Loop(1) = {1, 2, 3, 4};
Plane Surface(1) = {1};

// Setup physical domains with labels
Physical Surface("copper") = {1};

// Setup sidesets
Physical Line("walls") = {2, 4};
Physical Line("port") = {1};
Physical Line("exit") = {3};
