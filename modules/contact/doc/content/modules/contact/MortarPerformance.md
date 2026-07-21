# Mortar-Based Mechanical Contact Performance

Results of performance studies on the mortar-based contact approach are shown here.

## Friction formulation regression gate id=friction-formulation-regression-gate

The performance gate for issue `#32856` was constructed on the local
`contact-performance-benchmarks` branch from `up/next` commit
`7b332444532bd42dee0e095a07e0999fca53a136`. The final benchmark commit is
`27f5bc1e64655442a7724b16a2c6e560e948238e`. The
`automate-mortar-constants-32856` branch, originally at
`2edd1b1eb4b2de78e7ed32c257fd9adb17d002d0`, was rebased directly onto that commit. The tested
feature specialization is `56dad900eec5bece9256b6d9ad98e73ea77746a7`.

The suite contains three workloads and six `RunApp` candidates:

- `test/tests/mortar_performance.{hsw,ac}` is a controlled small-strain problem with 800
  alternating load cycles and 1600 fixed steps. Its geometry and material properties give the
  physical strategies \(C_n=C_t=1\). It exercises repeated slip reversal under a sustained normal
  preload and validates completion, fixed time steps, finite values, KKT and displacement
  equilibrium errors, solver work, and AC/HSW physical agreement from repository-local CSV data.
- `test/tests/pdass_problems.{hsw,ac}` extends the existing finite-strain frictional bouncing block
  to time 14, preserving its \(c=c_t=10\), tolerances, time step, and PJFNK configuration. MUMPS
  replaces only the LU factor package so that the workload is stable with MPI and threads.
- `test/tests/3d-mortar-contact.{hsw,ac}` preserves the existing three-dimensional finite-strain
  problem, \(c=c_t=10^4\), tolerances, time step, and PJFNK configuration. It also uses MUMPS for
  thread-stable LU factorization.

On the benchmark branch both candidate names execute the legacy degree-two friction residual with
distinct output bases. On the feature branch the names and workloads are unchanged, the analyst
cases retain their user constants, and each candidate selects
`hueber_stadler_wohlmuth` or `alart_curnier` explicitly. There are no duplicated test entries for
MPI, threads, or distributed meshes.

The existing finite-strain input in `mortar_tm/2d/ad_frictional` was rejected as a performance
workload because both labels reached the nonlinear iteration limit with two MPI ranks. The existing
frictional field-split input was rejected after both labels diverged at four ranks with a distributed
mesh. Refined variational candidates were also rejected for orientation failures. These failures
were not converted into allowed caveats: a workload that was brittle under an execution shape was
removed from the performance suite.

### Local execution and deterministic checks

All local runs used the `mpich-mpicc` conda environment and the following base command:

```bash
./run_tests -i performance --capture-perf-graph --sep-files
```

Characterization used separate output directories and result files for these launch shapes:

```bash
OMP_NUM_THREADS=1 ./run_tests -j1 -p1 --n-threads 1 ...
OMP_NUM_THREADS=1 ./run_tests -j2 -p2 --n-threads 1 ...
OMP_NUM_THREADS=1 ./run_tests -j4 -p4 --n-threads 1 --distributed-mesh ...
OMP_NUM_THREADS=2 ./run_tests -j2 -p1 --n-threads 2 ...
OMP_NUM_THREADS=2 ./run_tests -j4 -p2 --n-threads 2 --distributed-mesh ...
```

Here `-j` is the TestHarness total slot limit. It must be at least processes times threads; using
`-j1` with a multi-process or multi-thread launch skips rather than runs the test. The explicit
`OMP_NUM_THREADS` setting is required because an external factor package such as MUMPS does not use
the libMesh thread count as its OpenMP control.

Each final feature configuration was repeated three times. All 15 TestHarness invocations and all
90 candidate runs passed with no skips. The recorded environment has `OMP_NUM_THREADS=2` in every
threaded candidate. Every result contains a positive `runner_run` value and readable
`PerfGraphReporter` metadata. The final benchmark serial suite passed 6/6; each selected benchmark
workload also passed all five launch shapes during selection. The controlled threaded case was
repeated after setting its solver-work interval.

The controlled-case values observed over the final feature matrix were:

| Metric | Observed interval or limit |
| - | - |
| Cumulative nonlinear iterations | 7997--7998; committed upper budget 8000 |
| Residual evaluations | 9597--9598; committed upper budget 9600 |
| Diagnostic linear iterations | 7997--7999; committed upper budget 8001 |
| Maximum KKT error | \(5.53\times10^{-17}\) |
| Maximum displacement-equilibrium error | \(4.39\times10^{-13}\) |
| AC/HSW final pressure difference | at most \(10^{-15}\) |

The upper budgets include an earlier characterized two-thread observation of 8000 nonlinear,
9600 residual, and 8001 linear iterations. Only upper bounds are enforced so a future solver
robustness improvement is accepted. Solve failure, cutback, non-finite CSV data, KKT or equilibrium
error above \(10^{-10}\), or AC/HSW pressure difference above \(10^{-8}\) fails validation.

The rebuilt contact unit executable ran 30/30 tests successfully. This includes projection roots
and homogeneity, the corrected separation sign, two- and three-dimensional stick and slip,
separation and reclosure, reversal, multiple increments, initial-guess variation, user/physical
scale equivalence, row-scaling invariance, and the existing refinement and field-split checks. The
row-scaling invariant comparison uses \(10^{-12}\) nonlinear tolerances and
`abort_on_solve_fail=true`; this prevents the attenuated HSW residual from terminating before the
unchanged \(10^{-8}\) physical-pressure comparison is satisfied.

### Local timing and solver-work findings

The table compares one benchmark serial `runner_run` value with the range from four feature serial
runs. Local wall time is diagnostic only; the remote performance process is authoritative.

| Workload | Formulation | Benchmark (s) | Feature range (s) | Feature/base ratio |
| - | - | - | - | - |
| Bouncing block | HSW | 17.090 | 19.643--22.540 | 1.149--1.319 |
| Bouncing block | AC | 17.100 | 17.832--19.425 | 1.043--1.136 |
| 3-D mortar | HSW | 7.798 | 9.153--9.669 | 1.174--1.240 |
| 3-D mortar | AC | 7.708 | 4.478--4.811 | 0.581--0.624 |
| Controlled | HSW | 17.118 | 16.174--17.782 | 0.945--1.039 |
| Controlled | AC | 17.228 | 17.404--18.457 | 1.010--1.071 |

Serial solver-work diagnostics were:

| Workload | Candidate | Benchmark nonlinear / residual / linear | Feature nonlinear / residual / linear |
| - | - | - | - |
| Bouncing block | HSW | 224 / 280 / 1404 | 256 / 312 / 1719 |
| Bouncing block | AC | 224 / 280 / 1404 | 216 / 278 / 1575 |
| 3-D mortar | HSW | 21 / 22 / 27 | 25 / 26 / 34 |
| 3-D mortar | AC | 21 / 22 / 27 | 10 / 11 / 21 |
| Controlled | HSW and AC | 7997 / 9597 / 7997 | 7997 / 9597 / 7997 in serial |

The two analyst workloads begin from or traverse an open contact state. Their feature paths include
the corrected positive-separation gap sign, so their off-solution problems are not physically
equivalent to the benchmark and their counts are excluded from the solver-work acceptance
comparison. The physically equivalent unit-scale controlled case does not exceed the benchmark
solver-work budget.

#### HSW iteration-count ablation

A commit-boundary bisect and a five-factor ablation were run after the initial comparison. The
commit immediately before `bea3df39c01` (`4cd59426986`) exactly reproduced the benchmark counts for
both analyst workloads. Therefore none of the earlier physical-constant commits caused the
iteration changes; all relevant behavior entered in `bea3df39c01`. Because that commit combined
interacting changes, a diagnostic branch at `b65765f1bfd` exposed each behavior independently:

- corrected versus legacy augmented-normal-pressure gap sign;
- exact zero-degeneracy handling versus the legacy `contact_pressure < epsilon` cutoff;
- normalized versus raw degree-two HSW rows;
- PETSc right scaling versus no right scaling; and
- refactored versus direct user-constant arithmetic.

The direct MUMPS candidate commands were run with `OMP_NUM_THREADS=1`. A five-bit configuration
selected the preceding factors in order, with zero meaning feature behavior and one meaning legacy
behavior. The `00000` endpoint reproduced the feature counts, while `11111` reproduced the
benchmark counts exactly. Each row below shows the result when only that feature is introduced onto
the legacy endpoint and when only that feature is restored to legacy behavior from the feature
endpoint. Values are nonlinear iterations / residual evaluations / diagnostic linear iterations.

| Factor | Bouncing block: feature alone | 3-D: feature alone | Bouncing block: restored from feature | 3-D: restored from feature |
| - | - | - | - | - |
| Corrected gap sign | 219 / 275 / 1372 | 29 / 30 / 35 | 306 / 362 / 1900 | 26 / 27 / 35 |
| Exact open-state handling | 300 / 356 / 1870 | 27 / 28 / 36 | 174 / 247 / 1392 | 25 / 26 / 31 |
| HSW row normalization | 201 / 337 / 1969 | 21 / 22 / 27 | 259 / 321 / 1885 | 26 / 27 / 35 |
| PETSc right scaling | 214 / 270 / 1331 | 21 / 22 / 27 | 222 / 278 / 1640 | 25 / 26 / 34 |
| Refactored user-scale arithmetic | 212 / 268 / 1312 | 21 / 22 / 27 | 275 / 337 / 1996 | 25 / 26 / 34 |

The legacy cutoff removal is the dominant isolated regression and is sufficient to account for the
final bouncing-block increase: restoring only that cutoff improves all three work metrics below
the benchmark. Right scaling adds bouncing-block work in the complete feature interaction even
though it improves that case in isolation. The corrected sign and exact open-state handling each
increase 3-D work in isolation, while row normalization and refactored arithmetic are not final
regression causes because restoring either legacy behavior makes the complete feature case worse.
Consequently the result is interaction-dependent: an ordered commit bisect of these behaviors can
identify a different first bad commit depending on their order and is not a unique causal ranking.

PerfGraph shows that the local HSW wall-time signal follows additional solve work rather than one
new per-call hotspot. In the bouncing block, Jacobian assemblies increased from 224 to 256 and
residual-kernel calls from 1908 to 2287. In the 3-D workload, Jacobian assemblies increased from 21
to 25 and residual assemblies from 49 to 60. These findings make the local HSW signal worth checking
remotely even though it is not an equivalent-problem solver-work failure.

### Remote acceptance

The existing remote CI performance process must run the same `performance` suite for the
noise-aware wall-time decision. No `CIVET_BASE_SHA` override or external CIVET recipe change is
used. If remote CI reports a regression, rerun it once. A confirmed regression must be investigated
with the captured PerfGraph data and revised or reverted before merge.

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
residuals. The third column denotes the type of non-linear complementarity
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
