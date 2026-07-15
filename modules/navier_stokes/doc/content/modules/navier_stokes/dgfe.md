# Discontinuous Galerkin Finite Element Navier Stokes

MOOSE can assemble a compatible incompressible flow and temperature-transport
discretization using discontinuous finite element spaces. Unlike the
[hybrid CG-DG discretization](hcgdgfe.md), the pressure is also discontinuous,
so its momentum contribution requires numerical flux terms on element faces.

This page describes the tested channel-flow demonstration in
`dg.i`. It uses first-order discontinuous velocity,
piecewise-constant pressure, and piecewise-constant temperature on triangular
elements.

## Compatible flow and scalar transport

The concept of compatibility for coupled flow and transport was introduced by
[!citep](dawson2004compatible). A compatible discretization retains the
accuracy and conservation properties of the transport method when it uses a
discrete velocity field. In [!citep](cesmelioglu2022compatible) the authors proved that if a Stokes or
Navier-Stokes discretization is exactly mass conserving, which requires two properties

1. the velocity be divergence conforming, e.g. $\llbracket u_h\cdot n\rrbracket = 0$
   across element faces
2. that $\nabla \cdot V_h \subseteq Q_h$ where $V_h$ is the space of velocity functions
   and $Q_h$ is the space of pressure functions,

then the space of the transport functions is not tied to the space of the pressure
functions in order to be compatible. (The IP-HDG discretization in [inshdg.md]
is exactly mass conserving; the L-HDG qualification is discussed there.) However,
for non-free flow such as Darcy, the transport test space must be contained in the
pressure test space, $W_h \subseteq Q_h$. The same inclusion is also required, at
least in our experience, for a compatible free-flow discretization when mass
conservation is not exact. Intuitively this makes sense because the discretization
no longer satisfies the strong form of the mass conservation equation, but instead
only does so weakly against pressure test functions. Every transport test function
must therefore also be an admissible pressure test function for its discrete balance
to use the same mass conservation statement. Equal transport and pressure spaces are
sufficient but not necessary. In the example below, pressure and temperature both
use a constant `MONOMIAL` space and therefore satisfy this inclusion.

## Variables

The velocity components `u` and `v` use first-order `MONOMIAL` bases.
Pressure and temperature use constant `MONOMIAL` bases, so all four solution
fields are discontinuous across element faces.

!listing modules/navier_stokes/test/tests/finite_element/ins/compatibility/dg-stokes.i block=Variables

## Volume and interior-face terms

The base input assembles the Stokes, mass-continuity, and temperature-diffusion
terms. [MatDiffusion.md] supplies the element-volume diffusion terms and
[DGDiffusion.md] supplies their interior numerical fluxes. The pressure
gradient is integrated by parts: [PressureGradient.md] supplies the volume
term, while [INSPressureGradientDGKernel.md] uses the centered pressure to
supply the interior-face term. Mass continuity is assembled with
[ADConservativeAdvection](ConservativeAdvection.md) and [ADDGAdvection.md] by
advecting the constant quantity `-1` with the computed velocity.

!listing modules/navier_stokes/test/tests/finite_element/ins/compatibility/dg-stokes.i block=Kernels DGKernels

The including input adds the nonlinear momentum and temperature advection
terms. [ADConservativeAdvection](ConservativeAdvection.md) supplies the
element-volume terms, and [ADDGAdvection.md] supplies upwind numerical fluxes
on interior faces.

!listing modules/navier_stokes/test/tests/finite_element/ins/compatibility/dg.i block=Kernels/momentum_x_convection Kernels/momentum_y_convection Kernels/temperature_convection DGKernels/momentum_x_convection DGKernels/momentum_y_convection DGKernels/temperature_convection

## Boundary terms

External faces complete the weak forms. [DGFunctionDiffusionDirichletBC.md]
weakly imposes the velocity and temperature diffusion conditions,
[ADConservativeAdvectionBC](ConservativeAdvectionBC.md) supplies inlet and
outlet advective fluxes, and [INSPressureGradientBC.md] supplies the pressure
term on the inlet and wall boundaries.

!listing modules/navier_stokes/test/tests/finite_element/ins/compatibility/dg-stokes.i block=BCs

The including input adds the inlet and outlet momentum and temperature
advection conditions:

!listing modules/navier_stokes/test/tests/finite_element/ins/compatibility/dg.i block=BCs/advection_momentum_x_in BCs/advection_momentum_y_in BCs/advection_temperature_in BCs/advection_momentum_x_out BCs/advection_momentum_y_out BCs/advection_temperature_out

## Conservation demonstration

The demonstration uses a 10-by-2 triangular channel mesh with unit density, a
unit-mean parabolic inlet x-velocity, and unit inlet temperature. Materials
construct the velocity and momentum properties used by the conservative
kernels, and a steady Newton solve computes the coupled flow and temperature
fields.

The `dg` test compares the outlet side averages
against CSV gold values. Both the outlet x-velocity and outlet temperature are
one, matching their inlet values and demonstrating mass and energy conservation
for this configuration.

!listing modules/navier_stokes/test/tests/finite_element/ins/compatibility/dg-stokes.i block=Postprocessors
