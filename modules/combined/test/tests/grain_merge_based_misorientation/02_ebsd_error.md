[33mEBSD orientation data may not be reliable for grain 0
[39m[33mEBSD orientation data may not be reliable for grain 29
[39m[33mEBSD orientation data may not be reliable for grain 48
[39m[33mEBSD orientation data may not be reliable for grain 82
[39m[33mEBSD orientation data may not be reliable for grain 94
[39m[33mEBSD orientation data may not be reliable for grain 151
[39m[33mEBSD orientation data may not be reliable for grain 211
[39m[33mEBSD orientation data may not be reliable for grain 214
[39mthe done of GrainTrackerMerge

Finished Setting Up                                                                      [[33m  1.71 s[39m] [[33m  132 MB[39m]

Framework Information:
MOOSE Version:           git commit cc8b5b8c03 on 2023-03-19
LibMesh Version:         
PETSc Version:           3.16.6
SLEPc Version:           3.16.2
Current Time:            Sun Mar 19 16:56:13 2023
Executable Timestamp:    Sun Mar 19 16:56:00 2023

Parallelism:
  Num Processors:          35
  Num Threads:             1

Mesh: 
  Parallel Type:           replicated
  Mesh Dimension:          2
  Spatial Dimension:       2
  Nodes:                   
    Total:                 33489
    Local:                 1020
    Min/Max/Avg:           876/1020/956
  Elems:                   
    Total:                 33124
    Local:                 949
    Min/Max/Avg:           924/964/946
  Num Subdomains:          1
  Num Partitions:          35
  Partitioner:             metis

Nonlinear System:
  Num DOFs:                334890
  Num Local DOFs:          10200
  Variables:               { "gr0" "gr1" "gr2" "gr3" "gr4" "gr5" "gr6" "gr7" "gr8" "gr9" } 
  Finite Element Types:    "LAGRANGE" 
  Approximation Orders:    "FIRST" 

Auxiliary System:
  Num DOFs:                298481
  Num Local DOFs:          8612
  Variables:               "bnds" { "unique_grains" "var_indices" "phi1" "Phi" "phi2" "RGB_x" "RGB_y" 
                             "RGB_z" } 
  Finite Element Types:    "LAGRANGE" "MONOMIAL" 
  Approximation Orders:    "FIRST" "CONSTANT" 

Execution Information:
  Executioner:             Transient
  TimeStepper:             IterationAdaptiveDT
  TimeIntegrator:          BDF2
  Solver Mode:             Preconditioned JFNK
  PETSc Preconditioner:    hypre boomeramg 




    Performing Initial Adaptivity
      Finished Handling Mesh Changes                                                     [[33m  6.08 s[39m] [[33m  128 MB[39m]
      Handling Mesh Changes.                                                             [[33m 15.54 s[39m] [[33m  288 MB[39m]
      Adapting Mesh                                                                      [[33m 10.81 s[39m] [[33m  187 MB[39m]
      Handling Mesh Changes......                                                        [[33m 42.16 s[39m] [[33m  689 MB[39m]
      Finished Projecting Initial Solutions                                              [[33m  6.39 s[39m] [[33m    1 MB[39m]
      Adapting Mesh                                                                      [[33m 13.89 s[39m] [[33m  106 MB[39m]
      Handling Mesh Changes.....                                                         [[33m 41.35 s[39m] [[33m  102 MB[39m]
      Finished Projecting Initial Solutions                                              [[33m  5.02 s[39m] [[33m    0 MB[39m]
    Finished Performing Initial Adaptivity                                               [[33m150.82 s[39m] [[33m 1615 MB[39m]

[33mGrain #223 intersects Grain #227 (variable index: 2)
[39m[32m- Depth 0: Remapping grain #223 from variable index 2 to 9 whose closest grain (#176) is at a distance of 123.486

[39mFinished inside of GrainTracker



  Finished Performing Initial Setup                                                      [[33m152.60 s[39m] [[33m 1630 MB[39m]


Time Step 0, time = 0

Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
| time           | DOFs           | bnd_length     | dt             | grain_tracker  | ngrains        | run_time       |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   2.400000e+02 |   0.000000e+00 |   0.000000e+00 |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


  Finished Outputting                                                                    [[33m  5.85 s[39m] [[33m    2 MB[39m]


Time Step 1, time = 0.1, dt = 0.1
 0 Nonlinear |R| = [32m3.171144e+01[39m
 1 Nonlinear |R| = [32m9.624871e-01[39m
 2 Nonlinear |R| = [32m1.588126e-03[39m
 3 Nonlinear |R| = [32m5.578608e-08[39m

  Finished Solving                                                                       [[33m 22.13 s[39m] [[33m    7 MB[39m]

[32m Solve Converged![39m


Grain Tracker Status:
Grains active index 0: 59 -> 59
Grains active index 1: 41 -> 41
Grains active index 2: 36 -> 36
Grains active index 3: 33 -> 36++
Grains active index 4: 28 -> 28
Grains active index 5: 18 -> 18
Grains active index 6: 15 -> 15
Grains active index 7: 6 -> 6
Grains active index 8: 2 -> 2
Grains active index 9: 2 -> 2

[33mSplit Grain Detected #0 (variable index: 3)
[39m[33mSplit Grain Detected #0 (variable index: 3)
[39m[33mSplit Grain Detected #0 (variable index: 3)
[39m[33mSplit Grain Detected #0 (variable index: 3)
[39m[33mSplit Grain Detected #0 (variable index: 3)
[39m[33mSplit Grain Detected #0 (variable index: 3)
[39mFinished inside of GrainTracker



Outlier Variable Residual Norms:
  gr2: [33m2.502915e-08[39m

Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
| time           | DOFs           | bnd_length     | dt             | grain_tracker  | ngrains        | run_time       |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   2.400000e+02 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e-01 |   6.012072e+06 |   5.003382e+03 |   1.000000e-01 |   2.430000e+02 |   2.430000e+02 |   1.857853e+02 |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


  Finished Outputting                                                                    [[33m  6.06 s[39m] [[33m    0 MB[39m]
  Adapting Mesh
    Adapting Mesh                                                                        [[33m 14.48 s[39m] [[33m   25 MB[39m]
  Still Adapting Mesh......

  Finished Adapting Mesh                                                                 [[33m 56.96 s[39m] [[33m   28 MB[39m]



Time Step 2, time = 0.21, dt = 0.11
 0 Nonlinear |R| = [32m1.868843e+01[39m
 1 Nonlinear |R| = [32m2.581372e-01[39m
 2 Nonlinear |R| = [32m7.044795e-05[39m
 3 Nonlinear |R| = [32m3.014504e-09[39m

[32m Solve Converged![39m

  Finished Solving                                                                       [[33m 25.31 s[39m] [[33m    5 MB[39m]


Grain Tracker Status:
Grains active index 0: 59 -> 59
Grains active index 1: 41 -> 41
Grains active index 2: 36 -> 36
Grains active index 3: 36 -> 33--
Grains active index 4: 28 -> 28
Grains active index 5: 18 -> 18
Grains active index 6: 15 -> 15
Grains active index 7: 6 -> 6
Grains active index 8: 2 -> 2
Grains active index 9: 2 -> 2

[32mMarking Grain 0 as INACTIVE (variable index: 3)
[39m[32mMarking Grain 0 as INACTIVE (variable index: 3)
[39m[32mMarking Grain 0 as INACTIVE (variable index: 3)
[39mFinished inside of GrainTracker



Outlier Variable Residual Norms:
  gr1: [33m1.440484e-09[39m

Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
| time           | DOFs           | bnd_length     | dt             | grain_tracker  | ngrains        | run_time       |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   2.400000e+02 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e-01 |   6.012072e+06 |   5.003382e+03 |   1.000000e-01 |   2.430000e+02 |   2.430000e+02 |   1.857853e+02 |
|   2.100000e-01 |   5.855582e+06 |   4.826854e+03 |   1.100000e-01 |   2.400000e+02 |   2.430000e+02 |   2.782110e+02 |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


  Finished Outputting                                                                    [[33m  6.51 s[39m] [[33m  -10 MB[39m]
  Adapting Mesh
    Adapting Mesh.                                                                       [[33m 15.67 s[39m] [[33m   11 MB[39m]
  Still Adapting Mesh.......

  Finished Adapting Mesh                                                                 [[33m 66.72 s[39m] [[33m   23 MB[39m]



Time Step 3, time = 0.331, dt = 0.121
 0 Nonlinear |R| = [32m8.725172e+00[39m
 1 Nonlinear |R| = [32m9.825718e-02[39m
 2 Nonlinear |R| = [32m2.149978e-05[39m
 3 Nonlinear |R| = [32m1.413284e-10[39m

[32m Solve Converged![39m

  Finished Solving                                                                       [[33m 34.70 s[39m] [[33m   24 MB[39m]


Grain Tracker Status:
Grains active index 0: 59 -> 59
Grains active index 1: 41 -> 41
Grains active index 2: 36 -> 36
Grains active index 3: 33 -> 33
Grains active index 4: 28 -> 28
Grains active index 5: 18 -> 18
Grains active index 6: 15 -> 15
Grains active index 7: 6 -> 6
Grains active index 8: 2 -> 2
Grains active index 9: 2 -> 2

[33mGrain #22 and Grain #28 was merged (misor: 0.454124).
[39m[33mGrain #101 and Grain #108 was merged (misor: 0.428499).
[39m[33mGrain #68 and Grain #71 was merged (misor: 0.681307).
[39m[33mGrain #61 and Grain #62 was merged (misor: 0).
[39m[33mGrain #17 and Grain #41 was merged (misor: 0.601591).
[39m[33mSplit Grain (#22) detected on unmatched OPs (0, 5) attempting to remap to 0.
[39m[33mSplit Grain (#101) detected on unmatched OPs (0, 1) attempting to remap to 0.
[39m[33mSplit Grain (#17) detected on unmatched OPs (2, 4) attempting to remap to 2.
[39m[33mSplit Grain (#68) detected on unmatched OPs (2, 6) attempting to remap to 2.
[39m[33mSplit Grain (#61) detected on unmatched OPs (3, 4) attempting to remap to 3.
[39m[33mGrain #17 intersects Grain #63 (variable index: 4)
[39m[32m- Depth 0: Remapping grain #17 from variable index 4 to 9 whose closest grain (#176) is at a distance of 63.189

[39mFinished inside of GrainTracker



Outlier Variable Residual Norms:
  gr0: [33m6.600045e-11[39m

Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
| time           | DOFs           | bnd_length     | dt             | grain_tracker  | ngrains        | run_time       |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   2.400000e+02 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e-01 |   6.012072e+06 |   5.003382e+03 |   1.000000e-01 |   2.430000e+02 |   2.430000e+02 |   1.857853e+02 |
|   2.100000e-01 |   5.855582e+06 |   4.826854e+03 |   1.100000e-01 |   2.400000e+02 |   2.430000e+02 |   2.782110e+02 |
|   3.310000e-01 |   8.439443e+06 |   4.689038e+03 |   1.210000e-01 |   2.400000e+02 |   2.420000e+02 |   3.920112e+02 |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+



    Finished Outputting Step                                                             [[33m  8.43 s[39m] [[33m  -23 MB[39m]
  Finished Outputting                                                                    [[33m 10.83 s[39m] [[33m  -23 MB[39m]
  Adapting Mesh
    Adapting Mesh.                                                                       [[33m 17.89 s[39m] [[33m    4 MB[39m]
  Still Adapting Mesh......

  Finished Adapting Mesh                                                                 [[33m 62.78 s[39m] [[33m   62 MB[39m]



Time Step 4, time = 0.4641, dt = 0.1331
 0 Nonlinear |R| = [32m9.691704e+00[39m
 1 Nonlinear |R| = [32m9.308685e-02[39m
 2 Nonlinear |R| = [32m2.043386e-05[39m
 3 Nonlinear |R| = [32m3.420145e-10[39m

[32m Solve Converged![39m

  Finished Solving                                                                       [[33m 14.17 s[39m] [[33m    2 MB[39m]


Grain Tracker Status:
Grains active index 0: 57 -> 57
Grains active index 1: 42 -> 41--
Grains active index 2: 34 -> 34
Grains active index 3: 32 -> 32
Grains active index 4: 30 -> 26--
Grains active index 5: 19 -> 18--
Grains active index 6: 16 -> 15--
Grains active index 7: 6 -> 6
Grains active index 8: 2 -> 2
Grains active index 9: 2 -> 2

[32mMarking Grain 101 as INACTIVE (variable index: 1)
[39m[32mMarking Grain 17 as INACTIVE (variable index: 4)
[39m[32mMarking Grain 63 as INACTIVE (variable index: 4)
[39m[32mMarking Grain 56 as INACTIVE (variable index: 4)
[39m[32mMarking Grain 61 as INACTIVE (variable index: 4)
[39m[32mMarking Grain 22 as INACTIVE (variable index: 5)
[39m[32mMarking Grain 68 as INACTIVE (variable index: 6)
[39mFinished inside of GrainTracker



Outlier Variable Residual Norms:
  gr4: [33m2.000372e-10[39m

Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
| time           | DOFs           | bnd_length     | dt             | grain_tracker  | ngrains        | run_time       |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   2.400000e+02 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e-01 |   6.012072e+06 |   5.003382e+03 |   1.000000e-01 |   2.430000e+02 |   2.430000e+02 |   1.857853e+02 |
|   2.100000e-01 |   5.855582e+06 |   4.826854e+03 |   1.100000e-01 |   2.400000e+02 |   2.430000e+02 |   2.782110e+02 |
|   3.310000e-01 |   8.439443e+06 |   4.689038e+03 |   1.210000e-01 |   2.400000e+02 |   2.420000e+02 |   3.920112e+02 |
|   4.641000e-01 |   3.657345e+06 |   4.569076e+03 |   1.331000e-01 |   2.330000e+02 |   2.340000e+02 |   4.829210e+02 |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+


  Adapting Mesh
    Adapting Mesh.                                                                       [[33m 15.17 s[39m] [[33m   15 MB[39m]
  Still Adapting Mesh........

  Finished Adapting Mesh                                                                 [[33m 69.90 s[39m] [[33m   27 MB[39m]



Time Step 5, time = 0.61051, dt = 0.14641
 0 Nonlinear |R| = [32m5.153564e+00[39m
 1 Nonlinear |R| = [32m3.287335e-02[39m
 2 Nonlinear |R| = [32m7.703833e-06[39m
 3 Nonlinear |R| = [32m1.552537e-10[39m

  Finished Solving                                                                       [[33m 39.14 s[39m] [[33m   11 MB[39m]

[32m Solve Converged![39m


Grain Tracker Status:
Grains active index 0: 57 -> 57
Grains active index 1: 41 -> 41
Grains active index 2: 34 -> 34
Grains active index 3: 32 -> 32
Grains active index 4: 26 -> 26
Grains active index 5: 18 -> 18
Grains active index 6: 15 -> 15
Grains active index 7: 6 -> 6
Grains active index 8: 2 -> 2
Grains active index 9: 2 -> 2

Finished inside of GrainTracker



Outlier Variable Residual Norms:
  gr4: [33m8.331834e-11[39m

Postprocessor Values:
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
| time           | DOFs           | bnd_length     | dt             | grain_tracker  | ngrains        | run_time       |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   0.000000e+00 |   2.400000e+02 |   0.000000e+00 |   0.000000e+00 |
|   1.000000e-01 |   6.012072e+06 |   5.003382e+03 |   1.000000e-01 |   2.430000e+02 |   2.430000e+02 |   1.857853e+02 |
|   2.100000e-01 |   5.855582e+06 |   4.826854e+03 |   1.100000e-01 |   2.400000e+02 |   2.430000e+02 |   2.782110e+02 |
|   3.310000e-01 |   8.439443e+06 |   4.689038e+03 |   1.210000e-01 |   2.400000e+02 |   2.420000e+02 |   3.920112e+02 |
|   4.641000e-01 |   3.657345e+06 |   4.569076e+03 |   1.331000e-01 |   2.330000e+02 |   2.340000e+02 |   4.829210e+02 |
|   6.105100e-01 |   8.730536e+06 |   4.487273e+03 |   1.464100e-01 |   2.330000e+02 |   2.300000e+02 |   6.009871e+02 |
+----------------+----------------+----------------+----------------+----------------+----------------+----------------+



    Finished Outputting Step                                                             [[33m  7.45 s[39m] [[33m    0 MB[39m]
  Finished Outputting                                                                    [[33m 10.29 s[39m] [[33m    0 MB[39m]
      Finished Outputting Step                                                           [[33m  7.53 s[39m] [[33m    0 MB[39m]
    Finished Outputting                                                                  [[33m  7.53 s[39m] [[33m    0 MB[39m]
  Finished Executing Final Objects                                                       [[33m  7.54 s[39m] [[33m    0 MB[39m]
Finished Executing                                                                       [[33m618.11 s[39m] [[33m 1789 MB[39m]

