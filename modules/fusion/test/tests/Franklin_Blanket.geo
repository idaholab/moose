// Gmsh project created on Mon Jul 10 11:40:49 2023
SetFactory("OpenCASCADE");

R = 4.8; //major radius
a = 1.2; //minor radius
tau = 0.63; //triangularity
k = 2.2; //elongation
A = 0.5; //made up because I cant find that value no where

Point(1) = {0, 0, 0, 1.0};
Point(2) = {R, 0, 0, 1.0};
Point(3) = {R + a, 0, 0, 1.0};
Point(5) = {R - tau*a, k*a, 0, 1.0};
Point(6) = {R - tau*a, -k*a, 0, 1.0};
Point(7) = {R+A, 0, 0, 1.0};
Point(8) = {R - tau*A, k*A, 0, 1.0};
Point(9) = {R - tau*a, 0, 0, 1.0};
Point(10) = {R - tau*A, -k*A, 0, 1.0};
Point(11) = {R - tau*A, 0, 0, 1.0};

Ellipse(1) = {5, 9, 5, 3};
Ellipse(2) = {3, 9, 6, 6};
Ellipse(3) = {8, 11, 8, 7};
Ellipse(4) = {7, 11, 10, 10};
