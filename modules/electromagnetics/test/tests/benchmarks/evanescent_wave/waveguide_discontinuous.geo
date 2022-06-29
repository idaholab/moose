// Discontinuous waveguide mesh for evanescent wave decay benchmark
// Waveguide length = 4.5 cm
// Waveguide width (entry) = 1 cm
// Waveguide width (exit) = 0.5 cm
// Discontinuity location = 2.5 cm
// Entry source width = 0.5 cm
// Entry source height = 1 cm
// Entry location (lower left coordinate) = (0.0, 0.0)

// Use global scaling factor of 0.004 in order to reproduce benchmark mesh in
// documentation, and use a factor of 0.01 to reproduce the test mesh


// Define corners and important points in the domain (with scaling factors)
Point(1) = {-0, 0, 0, 1.0};
Point(2) = {0.005, 0, 0, 1.0};
Point(3) = {0.025, 0, 0, 1.0};
Point(4) = {0, 0.010, 0, 1.0};
Point(5) = {0.005, 0.010, 0, 1.0};
Point(6) = {0.045, 0.010, 0, 1.0};
Point(7) = {0.045, 0.005, 0, 1.0};
Point(8) = {0.025, 0.005, 0, 0.0025};

// Connect the outer points
Line(1) = {1, 4};
Line(2) = {4, 5};
Line(3) = {5, 6};
Line(4) = {6, 7};
Line(5) = {7, 8};
Line(6) = {8, 3};
Line(7) = {3, 2};
Line(8) = {2, 1};

// Connect the dividing line between the source region and general vacuum region
Line(9) = {2, 5};

// Define loop for source region
Line Loop(1) = {1, 2, -9, 8};
Plane Surface(1) = {1};

// Define loop for general vacuum region
Line Loop(2) = {3, 4, 5, 6, 7, 9};
Plane Surface(2) = {2};

// Setup physical domains with labels
Physical Surface("source") = {1};
Physical Surface("vacuum") = {2};

// Setup sidesets
Physical Line("walls") = {3, 2, 8, 7, 6, 5};
Physical Line("port") = {1};
Physical Line("exit") = {4};
