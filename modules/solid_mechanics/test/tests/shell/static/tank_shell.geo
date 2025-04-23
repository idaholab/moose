// Gmsh project created on Mon Mar 25 12:56:44 2024
SetFactory("OpenCASCADE");
//+
//+
//+

//+

//+
Circle(1) = {0, 0, 0, 0.5, 0, 2*Pi};
//+

//+
Transfinite Curve {1} = 20 Using Progression 1;
//+
Extrude {0, 0, 3} {
  Curve{1}; Layers {20}; Recombine;
}
//+
Physical Surface("shell", 4) = {1};
//+
Physical Curve("lower_circle", 5) = {1};
//+
Physical Curve("upper_circle", 6) = {3};
