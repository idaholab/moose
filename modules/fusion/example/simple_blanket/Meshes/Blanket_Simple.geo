// Gmsh project created on Tue Jun 27 14:24:33 2023
SetFactory("OpenCASCADE");

//////////////////////////////////Parameters///////////////////////////////////////
w = 0.35; // m, width of the single section of the blanket
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

////////////////First Wall Armor////////////////////
Point(1) = {-w/2, 0, 0, 1.0};
Point(2) = {w/2, 0, 0, 1.0};
Point(3) = {-w/2, d_shi, 0, 1.0};
Point(4) = {w/2, d_shi, 0, 1.0};

//////////////First Wall////////////////
Point(5) = {-w/2, d_shi+d_fw, 0, 1.0};
Point(6) = {w/2, d_shi+d_fw, 0, 1.0};

//////////////Multiplier Zone///////////
Point(7) = {-w/2, d_shi+d_fw+d_m, 0, 1.0};
Point(8) = {w/2, d_shi+d_fw+d_m, 0, 1.0};

Point(9) = {-w/2, d_shi+d_fw+d_m+d_tc, 0, 1.0};
Point(10) = {w/2, d_shi+d_fw+d_m+d_tc, 0, 1.0};

/////////////Breeder Zone///////////////
Point(11) = {-w/2, d_shi+d_fw+d_m+d_tc+d_b, 0, 1.0};
Point(12) = {w/2, d_shi+d_fw+d_m+d_tc+d_b, 0, 1.0};

Point(13) = {-w/2, d_shi+d_fw+d_m+d_tc+d_b+d_tc, 0, 1.0};
Point(14) = {w/2, d_shi+d_fw+d_m+d_tc+d_b+d_tc, 0, 1.0};

/////////Toridal Cooling Channels////////////
Translate {w_off, w_off, 0} {
  Duplicata { Point{7}; }
}
Translate {w_off, -w_off, 0} {
  Duplicata { Point{9}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{16}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{15}; }
}
Translate {w_ch+w_off, 0, 0} {
  Duplicata { Point{17}; Point{18}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{17}; Point{18}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{19}; Point{20}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{23}; Point{24}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{25}; Point{26}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{27}; Point{28}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{29}; Point{30}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{31}; Point{32}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{33}; Point{34}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{35}; Point{36}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{37}; Point{38}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{39}; Point{40}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{41}; Point{42}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{43}; Point{44}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{45}; Point{46}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{47}; Point{48}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{49}; Point{50}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{51}; Point{52}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{53}; Point{54}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{55}; Point{56}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{57}; Point{58}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{59}; Point{60}; }
}
Translate {w_off, 0, 0} {
  Duplicata { Point{61}; Point{62}; }
}
Translate {w_ch, 0, 0} {
  Duplicata { Point{63}; Point{64}; }
}
Translate {0, d_tc+d_b, 0} {
  Duplicata { Point{16}; Point{15}; Point{17}; Point{18}; Point{21}; Point{22}; Point{19}; Point{23}; Point{25}; Point{27}; Point{29}; Point{31}; Point{33}; Point{35}; Point{37}; Point{39}; Point{41}; Point{43}; Point{45}; Point{47}; Point{49}; Point{51}; Point{53}; Point{55}; Point{57}; Point{59}; Point{61}; Point{63}; Point{65}; Point{66}; Point{64}; Point{62}; Point{60}; Point{58}; Point{56}; Point{54}; Point{52}; Point{50}; Point{48}; Point{46}; Point{44}; Point{42}; Point{40}; Point{38}; Point{36}; Point{34}; Point{32}; Point{30}; Point{28}; Point{26}; Point{24}; Point{20}; }
}

/////////Lines////////////
Line(1) = {1, 2};
Line(2) = {2, 4};
Line(3) = {4, 3};
Line(4) = {3, 1};
Line(5) = {3, 5};
Line(6) = {5, 7};
Line(7) = {5, 6};
Line(8) = {6, 4};
Line(9) = {6, 8};
Line(10) = {8, 7};
Line(11) = {7, 9};
Line(12) = {9, 10};
Line(13) = {10, 8};
Line(14) = {16, 17};
Line(15) = {17, 18};
Line(16) = {18, 15};
Line(17) = {15, 16};
Line(18) = {21, 19};
Line(19) = {19, 20};
Line(20) = {20, 22};
Line(21) = {22, 21};
Line(22) = {23, 25};
Line(23) = {25, 26};
Line(24) = {26, 24};
Line(25) = {24, 23};
Line(26) = {27, 29};
Line(27) = {29, 30};
Line(28) = {30, 28};
Line(29) = {28, 27};
Line(30) = {31, 33};
Line(31) = {33, 34};
Line(32) = {34, 32};
Line(33) = {32, 31};
Line(34) = {35, 37};
Line(35) = {37, 38};
Line(36) = {38, 36};
Line(37) = {36, 35};
Line(38) = {39, 41};
Line(39) = {41, 42};
Line(40) = {42, 40};
Line(41) = {40, 39};
Line(42) = {43, 45};
Line(43) = {45, 46};
Line(44) = {46, 44};
Line(45) = {44, 43};
Line(46) = {47, 49};
Line(47) = {49, 50};
Line(48) = {50, 48};
Line(49) = {48, 47};
Line(50) = {51, 53};
Line(51) = {53, 54};
Line(52) = {54, 52};
Line(53) = {52, 51};
Line(54) = {55, 57};
Line(55) = {57, 58};
Line(56) = {58, 56};
Line(57) = {56, 55};
Line(58) = {59, 61};
Line(59) = {61, 62};
Line(60) = {62, 60};
Line(61) = {60, 59};
Line(62) = {63, 65};
Line(63) = {65, 66};
Line(64) = {66, 64};
Line(65) = {64, 63};
Line(66) = {9, 11};
Line(67) = {11, 12};
Line(68) = {12, 10};
Line(69) = {12, 14};
Line(70) = {14, 13};
Line(71) = {13, 11};
Line(72) = {67, 69};
Line(73) = {69, 70};
Line(74) = {70, 68};
Line(75) = {68, 67};
Line(76) = {71, 73};
Line(77) = {73, 118};
Line(78) = {118, 72};
Line(79) = {72, 71};
Line(80) = {74, 75};
Line(81) = {116, 75};
Line(82) = {117, 116};
Line(83) = {117, 74};
Line(84) = {76, 77};
Line(85) = {77, 114};
Line(86) = {114, 115};
Line(87) = {115, 76};
Line(88) = {78, 79};
Line(89) = {79, 112};
Line(90) = {112, 113};
Line(91) = {113, 78};
Line(92) = {80, 81};
Line(93) = {81, 110};
Line(94) = {110, 111};
Line(95) = {111, 80};
Line(96) = {82, 83};
Line(97) = {83, 108};
Line(98) = {108, 109};
Line(99) = {109, 82};
Line(100) = {84, 85};
Line(101) = {85, 106};
Line(102) = {106, 107};
Line(103) = {107, 84};
Line(104) = {86, 87};
Line(105) = {87, 104};
Line(106) = {104, 105};
Line(107) = {105, 86};
Line(108) = {88, 89};
Line(109) = {89, 102};
Line(110) = {102, 103};
Line(111) = {103, 88};
Line(112) = {90, 91};
Line(113) = {91, 100};
Line(114) = {100, 101};
Line(115) = {101, 90};
Line(116) = {92, 93};
Line(117) = {93, 98};
Line(118) = {98, 99};
Line(119) = {99, 92};
Line(120) = {94, 95};
Line(121) = {95, 96};
Line(122) = {96, 97};
Line(123) = {97, 94};

/////////Surfaces////////////
Curve Loop(1) = {1, 2, 3, 4};
Plane Surface(1) = {1};
Curve Loop(2) = {3, 5, 7, 8};
Plane Surface(2) = {2};
Curve Loop(3) = {7, 9, 10, -6};
Plane Surface(3) = {3};
Curve Loop(4) = {11, 12, 13, 10};
Curve Loop(5) = {64, 65, 62, 63};
Curve Loop(6) = {60, 61, 58, 59};
Curve Loop(7) = {56, 57, 54, 55};
Curve Loop(8) = {52, 53, 50, 51};
Curve Loop(9) = {48, 49, 46, 47};
Curve Loop(10) = {44, 45, 42, 43};
Curve Loop(11) = {40, 41, 38, 39};
Curve Loop(12) = {36, 37, 34, 35};
Curve Loop(13) = {32, 33, 30, 31};
Curve Loop(14) = {28, 29, 26, 27};
Curve Loop(15) = {24, 25, 22, 23};
Curve Loop(16) = {20, 21, 18, 19};
Curve Loop(17) = {15, 16, 17, 14};
Plane Surface(4) = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
Curve Loop(18) = {12, -68, -67, -66};
Plane Surface(5) = {18};
Curve Loop(19) = {71, 67, 69, 70};
Curve Loop(20) = {73, 74, 75, 72};
Curve Loop(21) = {79, 76, 77, 78};
Curve Loop(22) = {83, 80, -81, -82};
Curve Loop(23) = {87, 84, 85, 86};
Curve Loop(24) = {91, 88, 89, 90};
Curve Loop(25) = {95, 92, 93, 94};
Curve Loop(26) = {99, 96, 97, 98};
Curve Loop(27) = {103, 100, 101, 102};
Curve Loop(28) = {107, 104, 105, 106};
Curve Loop(29) = {111, 108, 109, 110};
Curve Loop(30) = {115, 112, 113, 114};
Curve Loop(31) = {119, 116, 117, 118};
Curve Loop(32) = {123, 120, 121, 122};
Plane Surface(6) = {19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

/////////Transfinite Stuff////////////
Transfinite Curve {70, 67, 12, 10, 7, 3, 1} = 30 Using Progression 1;
Transfinite Curve {71, 66, 11, 6, 5, 8, 9, 13, 68, 69} = 6 Using Progression 1;
Transfinite Curve {4, 2} = 2 Using Progression 1;
Transfinite Curve {62, 64, 58, 60, 54, 56, 50, 52, 46, 48, 42, 44, 38, 40, 34, 36, 30, 32, 26, 28, 22, 24, 18, 20, 14, 16, 72, 74, 78, 76, 80, 82, 86, 84, 88, 90, 94, 92, 98, 96, 102, 100, 106, 104, 110, 108, 112, 114, 116, 118, 120, 122} = 12 Using Progression 1;
Transfinite Curve {75, 73, 79, 77, 83, 81, 87, 85, 91, 89, 95, 93, 99, 99, 97, 103, 101, 107, 105, 111, 109, 115, 113, 119, 117, 123, 121, 17, 15, 21, 19, 25, 23, 29, 27, 33, 31, 37, 35, 41, 39, 45, 43, 49, 47, 53, 51, 57, 55, 61, 59, 65, 63} = 4 Using Progression 1;
Transfinite Surface {5};
Transfinite Surface {3};
Transfinite Surface {2};
Transfinite Surface {1};

/////////Extruding in the Z direction////////////
Extrude {0, 0, h} {
  Surface{6}; Surface{5}; Surface{4}; Surface{3}; Surface{2}; Surface{1}; Layers {3};
}

/////////Surface and Volume Labels////////////
Physical Surface("Heated_Surface", 365) = {132};
Physical Surface("CH1", 366) = {14, 13, 11, 12};
Physical Surface("CH2", 367) = {17, 18, 16, 15};
Physical Surface("CH3", 364) = {20, 19, 21, 22};
Physical Surface("CH4", 368) = {24, 26, 23, 25};
Physical Surface("CH5", 369) = {28, 27, 30, 29};
Physical Surface("CH6", 370) = {32, 31, 34, 33};
Physical Surface("CH7", 371) = {36, 37, 38, 35};
Physical Surface("CH8", 372) = {39, 40, 42, 41};
Physical Surface("CH9", 373) = {45, 46, 43, 44};
Physical Surface("CH10", 374) = {48, 47, 50, 49};
Physical Surface("CH11", 375) = {52, 51, 54, 53};
Physical Surface("CH12", 376) = {56, 57, 58, 55};
Physical Surface("CH13", 377) = {62, 59, 60, 61};
Physical Surface("CH14", 378) = {121, 122, 120, 119};
Physical Surface("CH15", 379) = {115, 118, 117, 116};
Physical Surface("CH16", 380) = {112, 113, 111, 114};
Physical Surface("CH17", 381) = {109, 108, 107, 110};
Physical Surface("CH18", 382) = {104, 105, 106, 103};
Physical Surface("CH19", 383) = {100, 99, 102, 101};
Physical Surface("CH20", 384) = {96, 95, 98, 97};
Physical Surface("CH21", 385) = {92, 91, 94, 93};
Physical Surface("CH22", 386) = {88, 89, 90, 87};
Physical Surface("CH23", 387) = {84, 83, 85, 86};
Physical Surface("CH24", 388) = {81, 80, 79, 82};
Physical Surface("CH25", 389) = {76, 77, 75, 78};
Physical Surface("CH26", 390) = {72, 71, 73, 74};
Physical Surface("Back_Wall", 396) = {10};

Physical Volume("Shield", 391) = {6};
Physical Volume("First_Wall", 392) = {5};
Physical Volume("Multiplier", 393) = {4};
Physical Volume("Toroidal_Plate", 394) = {1,3};
Physical Volume("Breeder", 395) = {2};

