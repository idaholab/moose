// ------------------------------
// 2D LES cavity (structured quads)
// Tunable near-wall clustering
// ------------------------------
SetFactory("OpenCASCADE");

// ---- Parameters you can tweak ----
L   = 1.0;        // cavity size (square: 0..L x 0..L)
Nx  = 240;        // cells along x (left<->right)
Ny  = 240;        // cells along y (bottom<->top)

// Geometric grading factors (>1 = smaller cells near the first point of each edge)
// We'll set opposite edges with reciprocal factors to get fine cells near BOTH walls.
rH  = 1.05;       // horizontal grading (x): bottom edge uses rH, top edge uses 1/rH
rV  = 1.05;       // vertical   grading (y): left   edge uses rV, right edge uses 1/rV

// Notes for LES:
// - Increase Nx/Ny or rH/rV (e.g., 1.02→1.08) to shrink first-cell size near walls (lower y+).
// - r ~ 1.02–1.08 is typical; start with 1.04–1.06 for moderate Re.

// ---- Geometry: unit square ----
Point(1) = {0, 0, 0};
Point(2) = {L, 0, 0};
Point(3) = {L, L, 0};
Point(4) = {0, L, 0};

Line(1) = {1, 2}; // bottom (x+)
Line(2) = {2, 3}; // right  (y+)
Line(3) = {3, 4}; // top    (x-)
Line(4) = {4, 1}; // left   (y-)

Curve Loop(1) = {1, 2, 3, 4};
Plane Surface(1) = {1};

// ---- Structured mesh with near-wall grading on all sides ----
// Opposite edges get reciprocal progression so each wall is finely resolved.
Transfinite Line{1} = Nx Using Progression rH;       // bottom: fine near x=0
Transfinite Line{3} = Nx Using Progression 1/rH;     // top:    fine near x=L
Transfinite Line{4} = Ny Using Progression rV;       // left:   fine near y=0
Transfinite Line{2} = Ny Using Progression 1/rV;     // right:  fine near y=L

Transfinite Surface{1} = {1,2,3,4};
Recombine Surface{1};                 // quads
Mesh.RecombineAll = 1;                // ensure quads

// Optional: second-order or keep linear
Mesh.ElementOrder = 1;                // 1=linear quads, 2=quadratic

// Optional: nicer node/element ordering
Mesh.SecondOrderIncomplete = 1;       // for quad P2 (if you switch to 2)

// ---- Physical groups (for BCs in your solver) ----
Physical Surface("Fluid") = {1};
Physical Curve("Wall_Bottom") = {1};
Physical Curve("Wall_Right")  = {2};
Physical Curve("Wall_Top_Lid")= {3};
Physical Curve("Wall_Left")   = {4};

// ---- Quality/robustness tweaks (safe defaults) ----
Mesh.Algorithm = 5;      // transfinite (ignored for the interior but safe)
Mesh.Optimize = 1;       // mild smoothing
