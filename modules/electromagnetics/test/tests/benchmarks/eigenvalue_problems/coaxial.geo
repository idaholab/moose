// Use global scaling factor = 0.4 to duplicate current saved MSH file.

SetFactory("OpenCASCADE");
Circle(1) = {0, 0, 0, 0.5, 0, 2*Pi};
Circle(2) = {0, 0, 0, 0.125, 0, 2*Pi};

Line Loop(1) = {1};
Line Loop(2) = {2};
Plane Surface(1) = {1, 2};

Physical Surface(1) = {1};
Physical Line("outer") = {1};
Physical Line("inner") = {2};
