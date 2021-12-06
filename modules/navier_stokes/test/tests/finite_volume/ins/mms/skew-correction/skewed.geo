lc1 = 0.7;
lc2 = 0.15;
lc3 = 0.7;
lc4 = 0.45;

Point(1) = {0.0, 0.0, 0, lc1};
Point(2) = {1.0, 0.0, 0, lc2};
Point(3) = {1.0, 1.0, 0, lc3};
Point(4) = {0.0, 1.0, 0, lc4};

Line(1) = {1, 2};
Line(2) = {3, 2};
Line(3) = {3, 4};
Line(4) = {4, 1};

Curve Loop(1) = {4, 1, -2, 3};

Plane Surface(1) = {1};

Physical Line("left") = {4};
Physical Line("right") = {2};
Physical Line("bottom") = {1};
Physical Line("top") = {3};
Physical Surface("domain") = {1};
