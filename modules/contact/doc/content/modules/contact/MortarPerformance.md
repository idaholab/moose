# Mortar-Based Mechanical Contact Performance

Results of performance studies on the mortar-based contact approach are shown here.

### Frictionless contact algorithm comparison id=frictionless_table

| Constraint | Displacement | NCP function | Time (arbitrary units) | Time steps | Nonlinear iterations |
| ---------- | ------------ | ------------ | ---------------------- | ---------- | -------------------- |
| Nodal | Mortar | Min | 4.164 | 40 | 104 |
| Nodal | Mortar | FB | 5.020 | 40 | 135 |
| Nodal | Nodal | Min | 3.124 | 41 | 104 |
| Nodal | Nodal | FB | 4.014 | 41 | 149 |
| Mortar | Mortar | Min | 4.461 | 40 | 106 |
| Mortar | Mortar | FB | 5.577 | 40 | 136 |
| Nodal | Nodal | RANFS | 2.700 | 40 | 99 |

The first column denotes the discretization algorithm used for applying the
frictionless contact constraints. Nodal denotes use of a `NodeFaceConstraint`;
`Mortar` denotes use of a `MortarConstraint`. The second column denotes the
discretization used for applying the contact forces to the displacement
residuals. The third column denotes the type of non-linear complimentarity
problem (NCP) function used to ensure that the contact constraints are
satisfied. Min indicates the canonical min function (see
[std::min](https://en.cppreference.com/w/cpp/algorithm/min)); FB represents the
Fischer-Burmeister function. `RANFS` denotes the Reduced Active Nonlinear
Function Set scheme in which no Lagrange Multipliers are used, and instead the
non-linear residual equations at the secondary nodes are replaced with the gap
function. The fourth column in the table is the simulation time in
arbitrary units (since timings will be different across machines). The fifth
column is the number of time steps required to reach the simulation end
time. The final, sixth column is the cumulative number of non-linear iterations
taken during the simulation (note that this does not include any non-linear
iterations from failed time steps).

Notes:

- Clearly having mortar mesh generation slows the simulation down, which is not surprising
- The min NCP function is undeniably better for solving normal contact
- For the pure nodal algorithms, the time step that did not converge featured classic ping-ponging behavior:

```
 5 Nonlinear |R| = 4.007951e-04
    |residual|_2 of individual variables:
                  disp_x:    0.000399808
                  disp_y:    2.75599e-05
                  normal_lm: 5.52166e-06


The number of nodes in contact is 11

      0 Linear |R| = 4.007951e-04
      1 Linear |R| = 1.287307e-04
      2 Linear |R| = 8.423398e-06
      3 Linear |R| = 1.046825e-07
      4 Linear |R| = 8.017310e-09
      5 Linear |R| = 3.053040e-10
  Linear solve converged due to CONVERGED_RTOL iterations 5
 6 Nonlinear |R| = 4.432193e-04
    |residual|_2 of individual variables:
                  disp_x:    0.000396694
                  disp_y:    0.00019545
                  normal_lm: 2.96013e-05


The number of nodes in contact is 11

      0 Linear |R| = 4.432193e-04
      1 Linear |R| = 1.355935e-04
      2 Linear |R| = 1.216010e-05
      3 Linear |R| = 6.386952e-07
      4 Linear |R| = 2.235594e-08
      5 Linear |R| = 2.884193e-10
  Linear solve converged due to CONVERGED_RTOL iterations 5
 7 Nonlinear |R| = 4.008045e-04
    |residual|_2 of individual variables:
                  disp_x:    0.000399816
                  disp_y:    2.76329e-05
                  normal_lm: 5.29313e-06


The number of nodes in contact is 11

      0 Linear |R| = 4.008045e-04
      1 Linear |R| = 1.287272e-04
      2 Linear |R| = 8.423081e-06
      3 Linear |R| = 1.047782e-07
      4 Linear |R| = 8.054781e-09
      5 Linear |R| = 3.046073e-10
  Linear solve converged due to CONVERGED_RTOL iterations 5
 8 Nonlinear |R| = 4.432194e-04
```

### Frictional contact algorithm comparison id=frictional_table

| LM normal | LM tangential | Displacement | NCP function normal | NCP function tangential | Time (arbitrary units) | Time steps | Nonlinear iterations | CLI PETSc options |
| --------- | ------------  | ------------ | ------------------- | ----------------------- | ---------------------- | ---------- | -------------------- | --- |
| Mortar | Mortar | Mortar | FB | FB | 8.241 | 40 | 175 | None |
| Mortar | Mortar | Mortar | Min | FB | 7.928 | 40 | 159 | None |
| Nodal | Mortar | Mortar | Min | FB | 7.459 | 40 | 152 | None |
| Mortar | Mortar | Mortar | Min | Min | 11.237 | 41 | 234 | None |
| Nodal | Nodal | Mortar | Min | Min | 39.409 | 55 | 275 | `-snes_ksp_ew 0` |
| Nodal | Nodal | Mortar | FB | FB | NA | NA | NA | None |

Notes:

- NA: solve did not converge
- Timings run on a different machine than the frictionless cases
- The most performant case uses a `NodeFaceConstraint` discretization for
  enforcing the normal contact conditions and `MortarConstraint` discretizations
  for enforcement of the Coulomb frictional constraints and application of
  forces to the displacement residuals. Interestingly, this performant case uses
  different NCP functions for normal and tangential constraints: `std::min` for
  the former and Fischer-Burmeister for the latter. This performant case is used
  for comparison with the node-face penalty algorithm, shown below:

### NCP-LM-Mortar vs Penalty-NodeFace

The table below compares the timing and solver performance of
[NCP-LM-Mortar](/in_and_out/constraint/frictional_lm.i) and
[Penalty-NodeFace](/in_and_out/constraint/frictional_04_penalty.i) algorithms. NCP-LM refers to use of an
NCP function for contact constraint enforcement on a lagrange multiplier. The
"Mortar" designation denotes that a mortar discretization is used for enforcing
the tangential Coulomb friction conditions and applying contact forces to the
displacement residuals.

| Algorithm | Time (arb. units) | Time steps to end time | Cumulative non-linear iterations |
| --- | --- | --- | --- |
| NCP-LM-Mortar | 13.901 | 151 | 476 |
| Penalty-NodeFace | 20.711 | 151 | 938 |

There's a cost associated with generation of the mortar segment mesh that
partially offsets the fact that the mortar case takes nearly half the non-linear
iterations of the penalty case.

## Petsc options for contact

Recommended PETSc options for use with mortar based frictional contact are:

```puppet
[Executioner]
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       1e-5          NONZERO               1e-15'
[]
```

Using Eisenstat-Walker is advantageous for frictional contact because time is
not wasted in the linear solve in early non-linear iterations while the contact
set and stick/slip conditions are being resolved. Later in the non-linear solve
when the set of constraints has been resolved, more linear iterations will be
used as the non-linear solver moves through the quadratic basin. Experience has
shown that a choice of 1e-5 for the matrix free finite differencing parameter
works well for many problems. However, the user may want to experiment with
values anywhere between 1e-8 and 1e-4 depending on their multi-physics. A
very small non-zero shift is used to avoid zero pivots during the LU
decomposition. This may be extraneous in many cases. Note that the Jacobian
entries for mortar based contact are accurate and complete enough that
incomplete factorization may be used in serial or as a sub-block solver for
block jacobi or additive schwarz in parallel. This may be necessary for large
problems where lu does not scale.

The recommended PETSc options for use with `NodeFaceConstraint` based contact are shown below :

```puppet
[Executioner]
  ...
  petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap
                        -ksp_gmres_restart'
  petsc_options_value = 'asm lu 20 101'
  ...
[../]
```

## Scaling effects

The effects of scaling on non-linear solve convergence are shown below for
different kinds of contact formulations. The results are based on the input files for
[RANFS](/ranfs-and-scaling/bouncing-block-ranfs.i),
[kinematic](/bouncing-block-kinematic.i), and
[tangential penalty](/bouncing-block-tan-pen.i),
running with two processes.

| Scheme | Cumulative nonlinear iterations | Cumulative linear iterations | Initial condition number | Variable Scaling Factor |
| - | - | - | - | - |
| SMP PJFNK RANFS no scaling AMG | 68 | 510 | 9e3 | 1 |
| SMP PJFNK RANFS residual auto-scaling AMG | 65 | 391 | 1e2 | 1.1e-2 |
| SMP PJFNK RANFS Jacobian auto-scaling AMG | 66 | 372 | 4e1 | 4.1e-4 |
| SMP PJFNK Kinematic no scaling AMG | 58 | 305 | 9e3 | 1 |
| SMP PJFNK Kinematic residual auto-scaling AMG | 58 | 305 | 1e2 | 1.1e-2 |
| SMP PJFNK Kinematic Jacobian auto-scaling AMG | 58 | 305 | 4e1 | 4.1e-4 |
| SMP PJFNK Tangential Penalty no scaling AMG | 65 | 400 | 9e3 | 1 |
| SMP PJFNK Tangential Penalty residual auto-scaling AMG | 65 | 400 | 1e2 | 1.1e-2 |
| SMP PJFNK Tangential Penalty Jacobian auto-scaling AMG | 65 | 400 | 4e1 | 4.1e-4 |
| FD PJFNK RANFS jacobian auto-scaling LU | 54 | 70 | 4e1 | 4.1e-4 |
| FD PJFNK Kinematic jacobian auto-scaling LU | 62 | 95 | 4e1 | 4.1e-4 |

Important takeaways:

- The solve efficiency of kinematic and tangential penalty formulations is independent of scaling (within the window of this problem)
- Because RANFS constraint residuals/Jacobians are of gap magnitude, the RANFS solve really does perform better when the internal physics is scaled to be on the same order of magnitude
- There are some bugs with the RANFS Jacobian functions because RANFS takes more nonlinear and linear iterations than kinematic, but with a FD Jacobian RANFS takes significantly less nonlinear and linear iterations than kinematic
