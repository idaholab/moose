// Bottom left
Point(0) = {0, 0, 0, .05};
// Top left
Point(1) = {0, 1, 0, .05};
// Bottom right
Point(2) = {.5, 0, 0, .05};
// Top right
Point(3) = {1, 1, 0, .05};

Line(4) = {0, 1};
Line(1) = {0, 2};
Line(2) = {2, 3};
Line(3) = {3, 1};

Line Loop(0) = {1, 2, 3, -4};
Plane Surface(0) = {0};
Physical Surface(0) = {0};
Physical Line("left") = {4};
Physical Line("bottom") = {1};
Physical Line("right") = {2};
Physical Line("top") = {3};
Physical Point("bottom_left") = {0};
Physical Point("top_right") = {3};

