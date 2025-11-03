Lx = 10;
Ly = 2;
Lstep = 1;
Lz = 1;
Point(1) = {0, Ly/2, 0, 1.0};
Point(2) = {0, Ly, 0, 1.0};
Point(3) = {Lx, Ly, 0, 1.0};
Point(4) = {Lx, 0, 0, 1.0};
Point(5) = {Lstep, 0, 0, 1.0};
Point(6) = {Lstep, Ly/2, 0, 1.0};

Point(11) = {0, Ly/2, Lz, 1.0};
Point(12) = {0, Ly, Lz, 1.0};
Point(13) = {Lx, Ly, Lz, 1.0};
Point(14) = {Lx, 0, Lz, 1.0};
Point(15) = {Lstep, 0, Lz, 1.0};
Point(16) = {Lstep, Ly/2, Lz, 1.0};


Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 5};
Line(5) = {5, 6};
Line(6) = {6, 1};

Line(11) = {11, 12};
Line(12) = {12, 13};
Line(13) = {13, 14};
Line(14) = {14, 15};
Line(15) = {15, 16};
Line(16) = {16, 11};

Line(101) = {1, 11};
Line(102) = {2, 12};
Line(103) = {3, 13};
Line(104) = {4, 14};
Line(105) = {5, 15};
Line(106) = {6, 16};

Line Loop(1) = {1, 102, -11, -101};
Plane Surface(1) = {1};
Line Loop(2) = {3, 104, -13, -103};
Plane Surface(2) = {2};
Line Loop(3) = {102, 12, -103, -2};
Plane Surface(3) = {3};
Line Loop(4) = {4, 105, -14, -104};
Plane Surface(4) = {4};
Line Loop(5) = {5, 106, -15, -105};
Plane Surface(5) = {5};
Line Loop(6) = {6, 101, -16, -106};
Plane Surface(6) = {6};
Line Loop(7) = {1, 2, 3, 4, 5, 6};
Plane Surface(7) = {7};
Line Loop(8) = {11, 12, 13, 14, 15, 16};
Plane Surface(8) = {8};

// Inflow
Physical Surface(1) = {1};
// Outflow
Physical Surface(2) = {2};
// Noslip
Physical Surface(3) = {3, 4, 5, 6, 7, 8};

Surface Loop(1) = {1, 2, 3, 4, 5, 6, 7, 8};
Volume(1) = {1};
Physical Volume(1) = {1};
