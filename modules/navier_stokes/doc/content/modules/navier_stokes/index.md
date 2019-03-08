# Navier-Stokes Module

The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the
Navier-Stokes equations using the continuous Galerkin finite element (CGFE) method. The Navier-Stokes
equations are usually solved using either the pressure-based, incompressible formulation (assuming a
constant fluid density), or the density-based, compressible formulation.

[The CGFE method](navier_stokes/cgfe.md) has been implemented to solve either the incompressible or
compressible Navier-Stokes equations. The original CGFE method is usually not numerically stable for
solving problems when the Peclet number is greater than 2. An SUPG (Streamline Upwind Petrov
Galerkin) scheme is implemented for stabilized solution in smooth compressible flows. A
low-diffusion, discontinuity/shock-capturing scheme is required but currently absent for the CGFE
method to obtain non-oscillatory solutions of flow problems that contain contact discontinuity or
shock waves. For compressible flow problems, users can choose the CGFE method only when the flow
field is sufficiently smooth.

For incompressible flow, we have implemented both pressure-stabilized
Petrov-Galerkin (PSPG) and streamline-upwind Petrov-Galerkin (SUPG) methods. The
former stabilization allows use of equal order shape functions by introducing an
on-diagonal dependence in the pressure equation, removing the saddle-point
nature of the problem. The latter SUPG method allows
simulation at much higher Reynolds numbers than if SUPG was not used. For an
overview of the incompressible Navier-Stokes capability, please see the journal
article
[here](https://www.sciencedirect.com/science/article/pii/S0965997817310591?via%3Dihub)
or the pre-print [here](https://arxiv.org/pdf/1710.08898.pdf). Note that
automatic differentiation versions of the incompressible objects have been
created; these objects are currently not as performant as their hand-coded
peers, but they can be used seamlessly in simulations with mesh deformation and
are guaranteed to generate correct Jacobians.

!syntax complete group=NavierStokesApp
