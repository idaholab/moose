# NavierStokesProblem

The `NavierStokesProblem` solves the saddle point incompressible Navier-Stokes
equations using Schur complement field split preconditioning. It focuses on the
Least Squares Commutator (LSC) preconditioner, also known as the BFBt
preconditioner, developed by Elman in [!citep](elman1999preconditioning) and
[!citep](elman2006block). The PETSc manual page on the LSC preconditioner can be
found [here](https://petsc.org/release/manualpages/PC/PCLSC/).

A linearized version of the incompressible Navier-Stokes equations discretized
with stable finite element pairings may be written as:

\begin{equation}
\begin{bmatrix}
A & B^T\\
B & 0
\end{bmatrix}
\begin{bmatrix}
u\\
p
\end{bmatrix}
=
\begin{bmatrix}
f\\
0
\end{bmatrix}
\end{equation}

where $u$ denotes velocity degrees of freedom, $p$ denotes pressure degrees of
freedom, and $f$ denotes the effect of body forces not dependent on the
velocity/pressure degrees of freedom (e.g. gravity). The LSC preconditioner
proposed by Elman approximates the Schur complement inverse via

\begin{equation}
\hat{S}^{-1} := (B\hat{M}_u^{-1}B^T)^{-1}B\hat{M}_u^{-1}A\hat{M}_u^{-1}B_T(B\hat{M}_u^{-1}B_T)^{-1}
\end{equation}

where $\hat{M}_u$ is a diagonal approximation of the velocity mass matrix. In
the case of continuous pressure elements (per [!citep](olshanskii2007pressure)),
the above can be re-written as

\begin{equation}
\hat{S}^{-1} := L_p^{-1}B\hat{M}_u^{-1}A\hat{M}_u^{-1}B^TL_p^{-1}
\end{equation}

where $L_p$ represents a Poisson-like operator with dimension corresponding to
the number of pressure degrees of freedom, hence the $p$ subscript. PETSc has
full support for LSC preconditioning. The PETSc user may optionally provide two
"auxiliary" matrices for LSC preconditioning, corresponding to $L_p$ and
$\hat{M}_u$, which correspond to the `NavierStokesProblem` parameters
[!param](/Problem/NavierStokesProblem/L_matrix) and
[!param](/Problem/NavierStokesProblem/mass_matrix) respectively. Note that in
order to have $\hat{M}_u^{-1}$ scaling, the PETSc option `-pc_lsc_scale_diag`
must be supplied. If `-pc_lsc_scale_diag` is not provided, then $\hat{M}_u$ is
implicitly the identity vector. If `-pc_lsc_scale_diag` is supplied and
[!param](/Problem/NavierStokesProblem/mass_matrix) is not provided, then
$\hat{M}_u$ will be populated with the diagonal of $A$. If
[!param](/Problem/NavierStokesProblem/L_matrix) is not provided, then it will be
computed automatically via

\begin{equation}
L_p = B\hat{M}_u^{-1}B^T
\end{equation}

[!citep](olshanskii2007pressure) proposed a modification to the LSC preconditioner:

\begin{equation}
\hat{S}^{-1} := \hat{M}_p^{-1}BL_u^{-1}AL_u^{-1}B^T\hat{M}_p^{-1}
\end{equation}

The Olshanskii preconditioner replaces the velocity mass matrix with a pressure
mass matrix ($\hat{M}_u \rightarrow \hat{M}_p$) and a Poisson-like operator of
pressure degree of freedom dimension with a Poisson-like operator of velocity
degree of freedom dimension ($L_p \rightarrow L_u$). The Olshanskii
preconditioning variant can be activated by setting the
[!param](/Problem/NavierStokesProblem/commute_lsc) to `true` and passing the
PETSc option `-pc_lsc_commute`. Note that if the commuted LSC preconditioner has
been requested, then [!param](/Problem/NavierStokesProblem/L_matrix) *must* be
provided since it cannot be formed from system matrix data. Similarly,
[!param](/Problem/NavierStokesProblem/mass_matrix) must also be provided.

A final option available to users is the [!param](/Problem/NavierStokesProblem/use_pressure_mass_matrix) parameter. For Stokes flow (no advective term in the momentum equation) it is known that the pressure mass matrix is spectrally equivalent to the Schur complement, in which case the pressure mass matrix is an ideal choice for forming a preconditioner. If the user sets this option to `true`, then

\begin{equation}
\hat{S}^{-1} := \hat{M}_p^{-1}
\end{equation}

in which case a "standard" preconditioner can be used (e.g. `-pc_type lu`,
`-pc_type hypre -pc_hypre_type boomeramg`, etc.) as opposed to LSC.

## Which preconditioner to choose

[!citep](zanetti2020scalable) compares the Elman and Olshanskii LSC
preconditioners for "high" Reynolds numbers. The overarching result from that
paper is that Olshanskii is better performing, although the largest kinematic
viscosity explored in that article direclty comparing the two methods is
$10^{-1}$. The Olshanskii preconditioner is known to be sub-optimal for high
viscosities. In the limit of Stokes flow in which the advective term is dropped,
the Schur complement preconditioned with Olshanskii has a condition number that
is the square of the optimally preconditioned (using the pressure mass matrix)
Schur complement, which leads roughly to a doubling in iterations for the Schur
complement solve (see [!citep](olshanskii2007pressure)). Below we compare three
different preconditioning methods for the Schur complement, Elman LSC,
Olshanskii LSC, and direct preconditioning of the Schur complement with the
pressure mass matrix, for three different kinematic viscosities and mesh sizes
for the lid driven cavity with a peak lid velocity of 1 and mesh dimension 1
(consequently the Reynolds number is the reciprocal of the kinematic
viscosity). The values shown in the table are the maximum number of linear
iterations observed during Krylov solves of the Schur complement during
the steady Newton solve.  For this test, the Elman preconditioner outperforms Olshanskii at
high viscosities. However, for low viscosities, Elman shows an iteration count
that scales with the mesh size whereas Olshanskii iteration counts are
independent of the mesh size. This is likely due to the better clustering of
eigenvalues by the Olshanskii preconditioner compared to Elman shown in
[!citep](zanetti2020scalable). Direct preconditioning with the pressure mass
matrix is very effective for high viscosities (as theory predicts), but performs
worse than Olshanskii (and Elman for smaller meshes) for low viscosities. In
summary, we recommend that users use Elman or the pressure mass matrix directly
for high viscosities (low Reynolds numbers) and Olshanskii for low viscosities
(high Reynolds numbers)

Elman:

| $nu$      | n = 20 | n = 40 | n = 80 |
| ---       | ---    | ---    | ---    |
| 100       | 5      | 6      | 8      |
| 1         | 5      | 7      | 8      |
| $10^{-2}$ | 13     | 16     | 22     |

Olshanskii:

| $nu$      | n = 20 | n = 40 | n = 80 |
| ---       | ---    | ---    | ---    |
| 100       | 11     | 12     | 12     |
| 1         | 11     | 12     | 12     |
| $10^{-2}$ | 17     | 18     | 17     |

Pressure

| $nu$      | n = 20 | n = 40 | n = 80 |
| ---       | ---    | ---    | ---    |
| 100       | 6      | 6      | 6      |
| 1         | 6      | 6      | 6      |
| $10^{-2}$ | 21     | 21     | 21     |

The tables above were created by running the `steady_vector_fsp_elman.i`,
`steady_vector_fsp.i`, and `steady_vector_fsp_stokes.i` inputs respectively. To
focus the comparison on the preconditioning of the Schur complement itself, all
multigrid preconditioners were replaced with LU decompositions (`-pc_type lu
-pc_factor_mat_solver_type mumps`). In the `steady_vector_fsp_stokes.i` input,
an [INSADMomentumAdvection.md] kernel was added as well.

!syntax parameters /Problem/NavierStokesProblem

!syntax inputs /Problem/NavierStokesProblem

!syntax children /Problem/NavierStokesProblem

!bibtex bibliography
