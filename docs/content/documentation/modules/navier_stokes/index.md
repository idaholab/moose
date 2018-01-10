#Navier-Stokes Module
The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the Navier-Stokes equations using the continuous Galerkin finite element (CGFE) method. The Navier-Stokes equations are usually solved using either the pressure-based, incompressible formulation (assuming a constant fluid density), or the density-based, compressible formulation.

[The CGFE method](navier_stokes/cgfe.md) has been implemented to solve either the incompressible or compressible Navier-Stokes equations. The original CGFE method is usually not numerically stable for solving problems when the Peclet number is greater than 2. An SUPG (Streamline Upwind Petrov Galerkin) scheme is implemented for stabilized solution in smooth compressible flows. A low-diffusion, discontinuity/shock-capturing scheme is required but currently absent for the CGFE method to obtain non-oscillatory solutions of flow problems that contain contact discontinuity or shock waves. For compressible flow problems, users can choose the CGFE method only when the flow field is sufficiently smooth.

 !syntax complete groups=navier_stokes
