# DSGShellMassEigenKernel

!syntax description /Kernels/DSGShellMassEigenKernel

## Description

This kernel provides the mass of one degree of freedom of a DSG shell element for an eigenvalue solve. Six kernels, one per `component`, give the mass matrix of the family: the consistent translational mass of the mid-surface, the density times the thickness, on the three displacements, and the rotary inertia of the thickness, $\rho t^3 / 12$, on the two rotations in the plane of the element through the projector $\mathbf{I} - \mathbf{e}_3 \mathbf{e}_3^T$. The drilling rotation is given no inertia, as in [InertialForceDSGShell](/InertialForceDSGShell.md), and the rotary inertia can be turned off with `rotary_inertia`.

The kernel is written with the sign convention of the eigenvalue solvers of the framework, as [MassEigenKernel](/MassEigenKernel.md) is, and must carry the eigen vector tag, `extra_vector_tags = 'eigen'`; it reports an error without it, since it would otherwise add a mass term to the residual of a static solve. The eigenvalues of the problem are then the squares of the circular frequencies, $\omega^2$. Three more things are needed in the input, shown in the example below: the stress divergence kernels of a static analysis, an [EigenDirichletBC](/EigenDirichletBC.md) beside the [DirichletBC](/DirichletBC.md) of every supported variable, and a preconditioner that assembles the full coupling between the displacements and the rotations ([SingleMatrixPreconditioner](/SingleMatrixPreconditioner.md) with `full = true`); without the latter the rotations would be missing from the eigenvectors. The mass matrix is singular because of the massless drilling rotation; the shift-and-invert spectral transformation of the [Eigenvalue](/Eigenvalue.md) executioner handles that.

The density is a material property, as for the inertia kernel. The verification case and its values are on the [verification page](/dsg_shell_vandv.md).

### Choosing the parameters

- `component`, `displacements` and `rotations` are as for [StressDivergenceDSGShell](/StressDivergenceDSGShell.md): six kernels, one per variable, each with `extra_vector_tags = 'eigen'`.
- `density` names the material property with the mass per unit volume of the shell material, as for the inertia kernel.
- `rotary_inertia` (default true) includes the rotary inertia $\rho t^3 / 12$ of the two in-plane rotations in the mass matrix; with false the eigenvalues come from the translational mass alone.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/dynamics/natural_frequencies.i block=Kernels/inertia_disp_z Kernels/inertia_rot_x BCs/eigen_clamp_disp_z Preconditioning Executioner Problem

!syntax parameters /Kernels/DSGShellMassEigenKernel

!syntax inputs /Kernels/DSGShellMassEigenKernel

!syntax children /Kernels/DSGShellMassEigenKernel
