lc = 0.25;
//+
Point(1) = {0, 0, 0, lc};
//+
Point(2) = {0.5, 0, 0, lc};
//+
Point(3) = {1, 1, 0, lc};
//+
Point(4) = {1, 4, 0, lc};
//+
Point(5) = {0, 4, 0, lc};
//+
Line(1) = {1, 2};
//+
Line(2) = {2, 3};
//+
Line(3) = {3, 4};
//+
Line(4) = {4, 5};
//+
Line(5) = {5, 1};
//+
Line Loop(1) = {3, 4, 5, 1, 2};
//+
Plane Surface(1) = {1};
//+
Physical Surface("volume") = {1};
//+
Physical Line("bottom") = {1};
//+
Physical Line("right") = {2, 3};
//+
Physical Line("left") = {5};
//+
Physical Line("top") = {4};
//+
Physical Point("top_right") = {4};
