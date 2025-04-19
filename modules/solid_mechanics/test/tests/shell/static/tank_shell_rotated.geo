// Gmsh project created on Mon Mar 25 12:56:44 2024
SetFactory("OpenCASCADE");
//+
//+
//+

//+

//+
Circle(1) = {0, 0, 0, 0.5, 0, 2*Pi};
//+
Rotate {{0, 1, 0}, {0, 0, 0}, Pi/4} {
  Curve{1}; 
}
//+
Transfinite Curve {1} = 20 Using Progression 1;
//+
Extrude {2.12, 0, 2.12} {
  Curve{1}; Layers {20}; Recombine;
}

//+



//+
Physical Curve("lower_circle", 4) = {1};
//+
Physical Curve("upper_circle", 5) = {3};
//+
Physical Surface("shell", 6) = {1};
