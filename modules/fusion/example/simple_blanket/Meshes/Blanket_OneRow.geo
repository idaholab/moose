// Gmsh project created on Thu Jun 29 10:15:10 2023
SetFactory("OpenCASCADE");

//////////////////////////////////Parameters///////////////////////////////////////
d_shi = 0.002; // m, depth of the first wall shield
d_fw = 0.038; // m, depth of the first wall
d_m = 0.023; // m, depth of the multiplier zone
d_b = 0.018; //m, depth of the breeder zone
d_tc = 0.022; //m, depth of the toridal channels

d_ch = 0.01; // m, depth of a cooling channel
w_ch = 0.02; // m, width of a cooling channel
w_off = 0.0067; // m, offset width of cooling channel
d_off = 0.0049; //m, offset depth of cooling channel

h = 0.0366; // m, height inbetween coolant channels on side wall
///////////////////////////////////////////////////////////////////////////////////
x = w_ch/2+w_off;

////////////////First Wall Armor////////////////////
Point(1) = {-x, 0, 0, 1.0};
Point(2) = {x, 0, 0, 1.0};
Point(3) = {-x, d_shi, 0, 1.0};
Point(4) = {x, d_shi, 0, 1.0};

//////////////First Wall////////////////
Point(5) = {-x, d_shi+d_fw, 0, 1.0};
Point(6) = {x, d_shi+d_fw, 0, 1.0};

//////////////Multiplier Zone///////////
Point(7) = {-x, d_shi+d_fw+d_m, 0, 1.0};
Point(8) = {x, d_shi+d_fw+d_m, 0, 1.0};

Point(9) = {-x, d_shi+d_fw+d_m+d_tc, 0, 1.0};
Point(10) = {x, d_shi+d_fw+d_m+d_tc, 0, 1.0};

/////////////Breeder Zone///////////////
Point(11) = {-x, d_shi+d_fw+d_m+d_tc+d_b, 0, 1.0};
Point(12) = {x, d_shi+d_fw+d_m+d_tc+d_b, 0, 1.0};

Point(13) = {-x, d_shi+d_fw+d_m+d_tc+d_b+d_tc, 0, 1.0};
Point(14) = {x, d_shi+d_fw+d_m+d_tc+d_b+d_tc, 0, 1.0};

/////////Toridal Cooling Channels////////////
Translate {w_off, w_off, 0} {
  Duplicata { Point{7}; }
}
Translate {0, d_ch, 0} {
  Duplicata { Point{15}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{16}; Point{15}; }
}
Translate {w_off, -w_off, 0} {
  Duplicata { Point{13}; }
}
Translate {0, -d_ch, 0} {
  Duplicata { Point{19}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{19}; Point{20}; }
}

/////////Lines////////////
Line(1) = {13, 11};
Line(2) = {11, 12};
Line(3) = {12, 14};
Line(4) = {14, 13};
Line(5) = {19, 21};
Line(6) = {21, 22};
Line(7) = {22, 20};
Line(8) = {20, 19};
Line(9) = {11, 9};
Line(10) = {10, 12};
Line(11) = {9, 10};
Line(12) = {17, 16};
Line(13) = {15, 16};
Line(14) = {18, 15};
Line(15) = {17, 18};
Line(16) = {9, 7};
Line(17) = {8, 7};
Line(18) = {8, 10};
Line(19) = {6, 5};
Line(20) = {5, 7};
Line(21) = {6, 8};
Line(22) = {5, 3};
Line(23) = {3, 1};
Line(24) = {1, 2};
Line(25) = {2, 4};
Line(26) = {4, 3};
Line(27) = {4, 6};

/////////Surfaces////////////
Curve Loop(1) = {1, 2, 3, 4};
Curve Loop(2) = {8, 5, 6, 7};
Plane Surface(1) = {1, 2};
Curve Loop(3) = {9, 11, 10, -2};
Plane Surface(2) = {3};
Curve Loop(4) = {16, -17, 18, -11};
Curve Loop(5) = {14, 13, -12, 15};
Plane Surface(3) = {4, 5};
Curve Loop(6) = {20, -17, -21, 19};
Plane Surface(4) = {6};
Curve Loop(7) = {22, -26, 27, 19};
Plane Surface(5) = {7};
Curve Loop(8) = {24, 25, 26, 23};
Plane Surface(6) = {8};

/////////Transfinite Stuff////////////
Transfinite Curve {4, 5, 7, 2, 11, 12, 14, 17, 19, 26, 24} = 5 Using Progression 1;
Transfinite Curve {1, 8, 6, 3, 9, 10, 16, 13, 15, 18, 20, 21, 22, 27} = 5 Using Progression 1;
Transfinite Curve {23, 25} = 2 Using Progression 1;
Transfinite Surface {2};
Transfinite Surface {4};
Transfinite Surface {5};
Transfinite Surface {6};
Recombine Surface "*";

/////////Extruding in the Z direction////////////
Extrude {0, 0, h} {
  Surface{1}; Surface{2}; Surface{3}; Surface{4}; Surface{5}; Surface{6}; Layers {3}; Recombine;
}

/////////Surface and Volume Labels////////////
Physical Surface("Heated_Surface", 77) = {36};
Physical Surface("CH1", 78) = {11, 12, 14, 13};
Physical Surface("CH2", 79) = {25, 26, 24, 23};
Physical Surface("Back_Wall", 80) = {10};

Physical Volume("Shield", 81) = {6};
Physical Volume("First_Wall", 82) = {5};
Physical Volume("Multiplier", 83) = {4};
Physical Volume("Toroidal_Plate", 84) = {3, 1};
Physical Volume("Breeder", 85) = {2};

////////Getting points for 1d channels//////////////+
Translate {w_ch/2, -d_ch/2, -h/2} {
  Duplicata { Point{28}; }
}

xyz[] = Point{45};
Printf("%g, %g, %g", xyz[0], xyz[1], xyz[2]);

h_point = xyz[2];
xyz2[] = Point{28};
h_diff = xyz2[2] - xyz[2];
h_add = h_diff/6;

For (0:5)
   h_point -= h_add;
   p = newp;
   Point(p) = {xyz[0], xyz[1], h_point, 1.0};
   Printf("%g", h_point);
EndFor

h_point_2 = xyz[2];
For (0:5)
   h_point_2 += h_add;
   p2 = newp;
   Point(p2) = {xyz[0], xyz[1], h_point_2, 1.0};
   Printf("%g", h_point_2);
EndFor

Translate {w_ch/2, -d_ch/2, -h/2} {
  Duplicata { Point{37}; }
}

xyz3[] = Point{58};
Printf("%g, %g, %g", xyz3[0], xyz3[1], xyz3[2]);

h3_point = xyz3[2];
For (0:5)
   h3_point -= h_add;
   p3 = newp;
   Point(p3) = {xyz3[0], xyz3[1], h3_point, 1.0};
   Printf("%g", h3_point);
EndFor

h_point_3 = xyz[2];
For (0:5)
   h_point_3 += h_add;
   p4 = newp;
   Point(p4) = {xyz3[0], xyz3[1], h_point_3, 1.0};
   Printf("%g", h_point_3);
EndFor

Translate {w_off+w_ch/2, w_off+d_ch/2, -h/2} {
  Duplicata { Point{23}; }
}

xyz4 = Point{71};
Printf("%g, %g, %g", xyz4[0], xyz4[1], xyz4[2]);

h4_point = xyz4[2];
For (0:5)
  h4_point += h_add;
  p5 = newp;
  Point(p5) = {xyz4[0], xyz4[1], h4_point, 1.0};
  Printf("%g", h4_point);
EndFor

h_point_4 = xyz4[2];
For (0:5)
   h_point_4 -= h_add;
   p6 = newp;
   Point(p6) = {xyz4[0], xyz4[1], h_point_4, 1.0};
   Printf("%g", h_point_4);
EndFor
