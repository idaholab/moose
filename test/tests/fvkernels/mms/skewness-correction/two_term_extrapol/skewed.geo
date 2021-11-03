lc1 = 0.33;
lc2 = 0.5;

Point(1) = {-1.0, 0.0, 0, lc2};
Point(2) = {0.0, 0.0,  0, lc1};
Point(3) = {0.0, 0.5, 0, lc1};
Point(4) = {-1.0,  0.5, 0, lc2};

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
