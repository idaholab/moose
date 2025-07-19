# Examples that use MFEM capabilities

## Description

As part of the MOOSE test suite, we solve a number of sample problems using MFEM
that may serve as a useful starting point for users to adapt:

## Thermal Problems

- [Steady State Diffusion](syntax/MFEM/SteadyStateDiffusion.md): Steady state diffusion problem
  for the temperature profile across a mug with fixed temperatures on two boundaries.

- [Transient Heat Transfer](syntax/MFEM/HeatTransfer.md): Transient heat conduction
  problem with a boundary parameterized by a heat transfer coefficient that exchanges
  heat with a thermal reservoir.

## Mechanical Problems

- [Linear Elasticity](syntax/MFEM/LinearElasticity.md): Solves a 3D linear elasticity
  problem for the deformation of a multi-material cantilever beam. This example
  is based on [MFEM Example 2](https://mfem.org/examples/#ex2).

## Electromagnetic Problems

- [Definite Maxwell](syntax/MFEM/DefiniteMaxwell.md): Solves a 3D electromagnetic
  diffusion problem for the electric field on a cube missing an octant, discretized
  using $H(\mathrm{curl})$ conforming Nédélec elements. This example is based on
  [MFEM Example 3](https://mfem.org/examples/#ex3).

- [Grad-div](syntax/MFEM/Grad-Div.md): Solves a diffusion problem for a vector field
  on a cuboid domain, discretized using $H(\mathrm{div})$ conforming Raviart-Thomas
  elements. This example is based on [MFEM Example 4](https://mfem.org/examples/#ex4) and
  is relevant for solving Maxwell's equations using potentials without the Coulomb gauge.

## Fluid Problems

- [Darcy](syntax/MFEM/Darcy.md): Solves a 2D mixed Darcy problem. This is a saddle point
  problem, discretized using Raviart-Thomas finite elements (velocity $\vec u$) and
  piecewise discontinuous polynomials (pressure $p$). This example demonstrates the use of
  transposition in the input file for mixed problems with different trial and test variables.
  Transposition flips the matrix associated a mixed bilinear form over its diagonal, swapping
  the roles of the test and trial variables and avoiding the need to define a second kernel.
  This example is based on [MFEM Example 5](https://mfem.org/examples/#ex5).
