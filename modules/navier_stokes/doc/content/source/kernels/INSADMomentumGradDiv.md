# INSADMomentumGradDiv

This kernel implements grad-div stabilization for the purpose of building
scalable preconditioners at high Reynolds numbers for Navier-Stokes as introduced in
[!citep](benzi2006augmented). Its weak form is given by

\begin{equation}
\gamma\left(\textrm{div}\ \mathbf{u}_h, \textrm{div}\ \mathbf{v}_h\right)
\end{equation}

where $\gamma$ is a penalty parameter, $\mathbf{u}_h$ is the velocity vector
approximate solution, and $\mathbf{v}_h$ are its associated test functions. For
$\gamma$ not too small, the Schur complement inverse of a linearized (via Newton or
Picard) stable (no pressure on-diagonal) Navier-Stokes discretization is well
approximated by

\begin{equation}
S^{-1} = -\left(\nu + \gamma\right)M_p^{-1}
\end{equation}

where $\nu$ is the kinematic viscosity and $M_p$ is the pressure mass matrix
([!citep](farrell2019augmented)). In [!citep](benzi2006augmented) the authors
found that $\gamma$ on the order of the velocity magnitude provides $\nu$- and
$h$-independent (the latter denoting element size) convergence for a wide range
of values for $\nu$ and $h$. For using the (scaled) pressure mass matrix as the
preconditioner for the Schur complement, the [NavierStokesProblem.md] should be
used in conjunction with [FieldSplitPreconditioner.md]. A demonstration of this
combination is given in the `steady_vector_fsp_al.i` input

!listing modules/navier_stokes/test/tests/finite_element/ins/lid_driven/steady_vector_fsp_al.i block=Problem

where we have indicated that we should use the pressure mass matrix as the
preconditioner for the Schur complement using the
[!param](/Problem/NavierStokesProblem/use_pressure_mass_matrix) parameter. The
scaled pressure mass matrix is built using the [MassMatrix.md] kernel with block

!listing modules/navier_stokes/test/tests/finite_element/ins/lid_driven/steady_vector_fsp_al.i block=Kernels/mass_kernel

The field split options are shown in

!listing modules/navier_stokes/test/tests/finite_element/ins/lid_driven/steady_vector_fsp_al.i block=Preconditioning

In this case we only wish to display the effectiveness of the pressure mass
matrix as a preconditioner for the Schur complement, so we use LU solves for
inverting the velocity block and $M_p$. Indeed for $\nu=10^{-3}$ (equivalent to
$\textrm{Re}=1000$), the solution of the Schur complement system to a relative tolerance of ($10^{-2}$) takes no more
than 4 Krylov iterations when running on grids ranging from 8x8 to
32x32. Scalable solves for the velocity block in the context of augmented
Lagrange formulations are the subject of [!cite](benzi2023solving).

!syntax parameters /Kernels/INSADMomentumGradDiv

!syntax inputs /Kernels/INSADMomentumGradDiv

!syntax children /Kernels/INSADMomentumGradDiv
