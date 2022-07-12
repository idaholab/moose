// Half-wave Dipole Antenna
// Resonant Frequency = 1 GHz
// wavelength = c / f
// Antenna length (L) = wavelength / 2
// Antenna feed gap = L / 20
// Domain radius = 5 * wavelength

// Use global scaling factor = 0.2 to duplicate the "fine" MSH file used for
// benchmark graphics.

SetFactory("OpenCASCADE");
Circle(1) = {0, 0, 0, 1.5, 0, 2*Pi};

Point(2) = {0, 0.15, 0, 0.01};

Point(3) = {0, -0.15, 0, 0.01};

Point(4) = {0, 0.0075, 0, 0.01};

Point(5) = {0, -0.0075, 0, 0.01};

Line(2) = {2, 4};

Line(3) = {5, 3};

Line Loop(1) = {1};

Plane Surface(1) = {1};
Line{2} In Surface{1};  // Tells Gmsh that the antenna poles are within the meshed surface (and
Line{3} In Surface{1};  // should have nodes on them)

Physical Line("boundary") = {1};

Physical Line("antenna") = {2, 3};

Physical Surface("vacuum") = {1};
