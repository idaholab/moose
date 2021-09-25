// Use global scaling factor = 0.12 to duplicate current saved MSH file.

width = 10;
length = 80;

Point(1) = {0, 0, 0};
Point(2) = {length, 0, 0};
Point(3) = {length, width, 0};
Point(4) = {0, width, 0};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};
Line Loop(5) = {1, 2, 3, 4};

Plane Surface(6) = {5};

Physical Line("bottom") = {1};
Physical Line("exit") = {2};
Physical Line("top") = {3};
Physical Line("port") = {4};

Physical Surface(7) = {6};
