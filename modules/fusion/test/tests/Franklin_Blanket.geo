// Gmsh project created on Mon Jul 10 11:40:49 2023
SetFactory("OpenCASCADE");

R = 0.7;
r_d = 0.5;
r = 0.5;
angle_h = 80*(180/Pi);
angle_r = 80*(180/Pi);

Point(1) = {0, 0, 0, 1.0};
Point(2) = {r*Sin(angle_h)+r_d, r*Cos(angle_h), 0, 1.0};
Point(3) = {r*Sin(angle_h)+r_d, -r*Cos(angle_h), 0, 1.0};
Point(4) = {R*Sin(angle_r)+r_d, R*Cos(angle_r), 0, 1.0};
Point(5) = {R*Sin(angle_r)+r_d, -R*Cos(angle_r), 0, 1.0};

Rotate {{0, 1, 0}, {0, 0, 0}, Pi/4} {
    Duplicata { Point{4}; Point{2}; Point{3}; Point{5}; }
}

Circle(1) = {8, 1, 7};
Circle(2) = {9, 1, 6};
Circle(3) = {3, 1, 2};
Circle(4) = {4, 1, 5};
Line(5) = {5, 3};
Line(6) = {8, 9};
Line(7) = {2, 4};
Line(8) = {7, 6};

Point(20) = {-0.3, r*Cos(angle_h), 0, 1.0};
Point(21) = {-0.3, -r*Cos(angle_h), 0, 1.0};
Point(22) = {-0.3, R*Cos(angle_h), 0, 1.0};
Point(23) = {-0.3, -R*Cos(angle_h), 0, 1.0};

Rotate {{0, 1, 0}, {0, 0, 0}, Pi/8} {
    Point{23}; Point{21}; Point{20}; Point{22}; 
}
  

Circle(9) = {8, 21, 3};
Circle(10) = {9, 23, 5};
Circle(11) = {7, 20, 2};
Circle(12) = {6, 22, 4};
Curve Loop(1) = {6, 10, 5, -9};
Surface(1) = {1};
Curve Loop(3) = {10, -4, -12, -2};
Surface(2) = {3};
Curve Loop(5) = {1, 11, -3, -9};
Surface(3) = {5};
Curve Loop(7) = {8, 12, -7, -11};
Surface(4) = {7};
Curve Loop(9) = {2, -8, -1, 6};
Surface(5) = {9};
Curve Loop(11) = {5, 3, 7, 4};
Surface(6) = {11};
Surface Loop(1) = {3, 1, 6, 2, 4, 5};
Volume(1) = {1};

Transfinite Curve "*" = 10;
