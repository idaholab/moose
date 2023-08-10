// Gmsh project created on Mon Jul 10 11:40:49 2023
SetFactory("OpenCASCADE");

R = 4.8; //major radius
a = 1.2; //minor radius
tau = 0.63; //triangularity
k = 2.2; //elongation
A = a - 0.9; //made up because I cant find that value no where

Point(1) = {0, 0, 0, 1.0};
Point(2) = {R, 0, 0, 1.0};
Point(3) = {R + a, 0, 0, 1.0};
Point(5) = {R - tau*a, k*a, 0, 1.0};
Point(6) = {R - tau*a, -k*a, 0, 1.0};
Point(7) = {R+A, 0, 0, 1.0};
Point(8) = {R - tau*a, k*a - 0.9, 0, 1.0};
Point(9) = {R - tau*a, 0, 0, 1.0};
Point(10) = {R - tau*a, -k*a + 0.9, 0, 1.0};


Ellipse(1) = {5, 9, 3, 3};
Ellipse(2) = {3, 9, 3, 6};
Ellipse(3) = {8, 9, 7, 7};
Ellipse(4) = {7, 9, 7, 10};

Line(5) = {5, 8};
Line(6) = {7, 3};
Line(7) = {10, 6};
Curve Loop(1) = {5, 3, 6, -1};
Plane Surface(1) = {1};
Curve Loop(2) = {6, 2, -7, -4};
Plane Surface(2) = {2};

Transfinite Curve {4, 2, 3, 1} = 6 Using Progression 1;
Transfinite Curve {5, 6, 7} = 3 Using Progression 1;
Extrude {{0, 1, 0}, {0, 0, 0}, 11.25*(Pi/180)} {
  Surface{1}; Surface{2}; Layers{3}; Recombine;
}

Extrude {{0, 1, 0}, {0, 0, 0}, -11.25*(Pi/180)} {
  Surface{1}; Surface{2}; Layers{3}; Recombine;
}

Physical Volume("Blanket", 36) = {3, 1, 2, 4};
Physical Surface("front", 37) = {4, 13, 19, 10};
Physical Surface("back", 38) = {15, 6, 8, 17};
Physical Surface("sides", 39) = {16, 20, 7, 11};
Physical Surface("ends", 40) = {3, 12, 18, 9};
