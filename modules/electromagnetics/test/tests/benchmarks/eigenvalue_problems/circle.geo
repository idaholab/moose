// Use global scaling factor = 0.5 to duplicate current saved MSH file.

SetFactory("OpenCASCADE");
Circle(1) = {0, 0, 0, 1, 0, 2*Pi};

Line Loop(1) = {1};
Plane Surface(1) = {1};

Physical Surface(1) = {1};
Physical Line("outer") = {1};
