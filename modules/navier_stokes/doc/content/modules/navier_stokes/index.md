# Navier-Stokes Module

The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the
Navier-Stokes equations using the continuous Galerkin finite element (CGFE) method. The Navier-Stokes
equations are usually solved using either the pressure-based, incompressible formulation (assuming a
constant fluid density), or the density-based, compressible formulation.

For documentation specific to finite element or finite volume implementations,
please refer to the below pages:

- [Continuous Galerkin Finite Element](navier_stokes/cgfe.md)
- [Finite Volume](navier_stokes/fv.md)
- [Turbulence Modeling Theory](navier_stokes/rans_theory.md)

!syntax complete groups=NavierStokesApp
