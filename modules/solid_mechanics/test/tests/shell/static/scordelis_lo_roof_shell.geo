// Gmsh project created on Mon Mar 25 12:56:44 2024
SetFactory("OpenCASCADE");
//+
//+
//+
Point(1) = {0, 0, 0, 1};
//+
Point(2) = {16.7, 0, 19.2, 1};
//+
Point(3) = {-16.7, 0, 19.2, 1};

//+
Circle(1) = {2, 1, 3};
//+
Extrude {0, 25, 0} {
  Curve{1}; 
}
//+
Physical Curve("left", 5) = {3};
//+
Physical Curve("right", 6) = {2};
//+
Physical Curve("back", 7) = {4};
//+
Physical Curve("front", 8) = {1};
//+
Physical Surface("shell", 9) = {1};
//+
Transfinite Surface {1};
//+
Transfinite Curve {4, 3, 1, 2} = 15 Using Progression 1;
//+
Recombine Surface {1};
