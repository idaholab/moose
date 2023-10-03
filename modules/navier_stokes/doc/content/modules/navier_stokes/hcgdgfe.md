# Hybrid Continuous/Discontinuous Galerkin Finite Element Navier Stokes

In [!citep](pandare2016hybrid) the authors describe a scheme in which the
velocity is discretized using a reconstructed discontinuous Galerkin (DG) method
while the pressure is discretized using continuous Galerkin (CG) finite
elements. While MOOSE has the ability to perform rDG(P0P1), otherwise known as a
second order accurate finite volume method (see [insfv.md]), it does not support
arbitrarily high-order reconstructions. It does, on the other hand, support
arbitrarily high-order discontinuous Galerkin bases. In that vein we have
implemented and tested a hybrid CG-DG scheme. A nice advantage of the hybrid
CG-DG scheme is that it is intrinsically LBB stable, allowing equal polynomial
order bases for the velocity and pressure [!citep](pandare2016hybrid).
Additionally, because the velocity is discretized with a DG scheme, the momentum
advection flux can be naturally upwinded without a complex upwinding scheme like
as Streamline-Upwind Petrov-Galerkin (SUPG) (see [cgfe.md]).

In the following we run through an example of a hybrid CG-DG input, in which we
are using the scheme to solve a lid driven cavity problem with a Reynolds number
of 200.

The first block is for the mesh. We create a simple square and add a nodeset
which we will use to pin the pressure, removing its nullspace (the
discretization only involves the gradient of pressure so without something like
a pressure pin, the pressure is only defined up to a constant).

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=Mesh

We next introduce the variables. `u` is the x-velocity component while `v` is
the y-velocity component. Note that we use a `MONOMIAL` basis but we could just
as well have used `L2_LAGRANGE` or `L2_HIERARCHIC`. Note that the default basis
order is 1 or `FIRST`. Similarly the default `family` is `LAGRANGE` so the
`pressure` variable is implicitly `LAGRANGE`. We note that a `MONOMIAL` basis
does not have the cross-terms that `L2_LAGRANGE` or `L2_HIERARCHIC` bases do for
non-simplicial elements; consequently for non-simplicial elements it is less
expensive to solve while having a worse error constant. Additionally, a
`MONOMIAL` basis has worse conditioning than `L2_LAGRANGE` or `L2_HIERARCHIC`
although that difference may not be appreciable until high polynomial
orders. As a reference in this test case, the condition number with a
5x5 mesh is 741 with a `MONOMIAL` basis for velocity and 265 for
`L2_HIERARCHIC` and `L2_LAGRANGE`. We note also that there is no
limitation on the family pairings for the hybrid CG-DG scheme so long as the
pressure variable is continuous and the velocity variable is
discontinuous. Finally, error convergence rates in the L2 norm for an equal order basis
of degree $n$ will be $n + 1$ for velocity and $n$ for pressure. The same
convergence rates will be observed with a basis of degree $n$ for velocity and
$n - 1$ for pressure: $n + 1$ in the L2 norm for velocity and $n$ in the L2 norm
for pressure.

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=Variables

We then add the `Kernels` block which adds the parts of the finite element weak
form terms that are integrated over element volumes. The first three kernels
comprise the x-momentum equation advection, diffusion, and pressure terms and
are of type [ADConservativeAdvection.md], [MatDiffusion.md], and
[PressureGradient.md] respectively. The next three kernels are the same physics
but for the y-momentum equation. The final kernel, `mass`, reuses the
[ADConservativeAdvection.md] kernel but is applied to the mass continuity
equation which is applied to the `pressure` variable.

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=Kernels

The `DGKernels` section adds the weak form terms corresponding to integrations
over internal faces. These only exist for the momentum equation since the
pressure is discretized with a continuous basis. The first two `DGKernels` are
for the x-momentum equation's advection and diffusion fluxes and are of type
[ADDGAdvection.md] and [DGDiffusion.md] respectively. The last two `DGKernels`
in the block are the same physics added for the y-momentum equation.

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=DGKernels

The `BCs` section adds the weak form terms which are integrated over external
boundary faces. Diffusive fluxes are applied using the
[DGFunctionDiffusionDirichletBC.md] object. The desired value of the solution on
the boundary is set using the `function` parameter. In `u_walls` we weakly
impose the condition that the x-velocity (`u`) is 0 on all but the top
boundary. In `vu_walls` we weakly impose the condition that the y-velocity (`v`)
is 0 on all boundaries. The impact of the lid is applied in `u_top` where we
impose a velocity of `${U}` which is 1. Here we point out a final advantage of
the DG scheme for velocity is that we can naturally impose discontinuous
boundary conditions, e.g. at the corners where the lid meets the walls there is
a discontinuity in what we weakly set for the velocity. The final boundary
condition in the `BCs` block, `pressure_pin`, imposes the constraint the
pressure be 0 at the `pinned_node`.

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=BCs

The `Materials` block defines properties necessary to complete the
simulation. These include the density, `rho`, the viscosity, `mu`, the vector
velocity, `velocity`, the x-momentum, `rhou`, and the y-momentum, `rhov`.

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=Materials

The `AuxVariables` and `AuxKernels` blocks are added for ease of comparison with
MOOSE finite volume results in Paraview.

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=AuxVariables AuxKernels

The simulation execution is defined in the `Executioner` block. We are
performing a steady-state calculation as indicated by `type = Steady`. Because
the Jacobian is completely accurate we use `solve_type = NEWTON` instead of
`solve_type = PJFNK`. In this case we are solving with a direct solver via
`-pc_type lu`. More sophisticated preconditioning techniques can be constructed
using [field splits](syntax/Preconditioning/index.md).

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=Executioner

We request [Exodus.md] output in the `Outputs` block

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=Outputs

Finally, we define a [ParsedPostprocessor.md] which prints the Reynolds number to
console output

!listing modules/navier_stokes/test/tests/finite_element/ins/cg-dg-hybrid/lid-driven/hybrid-cg-dg.i block=Postprocessors
