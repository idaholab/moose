#Navier-Stokes Module
The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the Navier-Stokes equations using either the continuous Galerkin finite element (CGFE) method or a reconstructed discontinuous Galerkin (rDG) method. The Navier-Stokes equations are usually solved using either the pressure-based, incompressible formulation (assuming a constant fluid density), or the density-based, compressible formulation.

Two spatial discretization methods are available for choice in this module:

1. [The CGFE method](navier_stokes/cgfe.md) has been implemented to solve either the incompressible or compressible Navier-Stokes equations. The original CGFE method is usually not numerically stable for solving problems when the Peclet number is greater than 2. An SUPG (Streamline Upwind Petrov Galerkin) scheme is implemented for stabilized solution in smooth compressible flows. A low-diffusion, discontinuity/shock-capturing scheme is required but currently absent for the CGFE method to obtain non-oscillatory solutions of flow problems that contain contact discontinuity or shock waves. For compressible flow problems, users can choose the CGFE method only when the flow field is sufficiently smooth.

2. [The rDG method](navier_stokes/rdg.md) has been implemented to solve the compressible Euler equations (when the viscous terms are neglected in the compressible Navier-Stokes equations). Extension of the rDG method for the compressible Navier-Stokes equations is completely durable. The specific rDG method implemented in this module is termed rDG(P0P1), which is equivalent to the second-order cell-centered finite volume method (FVM). To avoid ambiguity, the term "rDG" should be understood as "cell-centered FVM" in this module, and these two terms are used interchangeably in the following text. A separate [rDG module](/docs/content/documentation/modules/rdg/index.md) is created to host all the basic classes for supporting the implementation of a specific system of equations using the rDG method, e.g., the compressible Euler equations in this module.

 For questions regarding the CGFE implementation of the Navier-Stokes equations, contact John Peterson (<jw.peterson@inl.gov>).

 For questions regarding the rDG implementation of the compressible Euler equations, contact Yidong Xia (<yidong.xia@inl.gov>).




