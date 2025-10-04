# LinearFVMomentumBuoyancy

This kernel adds the contributions of the buoyancy force due to density differences through a force/source term to the right hand side of the momentum equation system for the finite volume SIMPLE segregated solver [SIMPLE.md].

This term is described by $(\rho-\rho_{ref}) \vec{g}$ present in the momentum conservation equation when describing an incompressible fluid, where $\rho_{ref}$ is the reference density, and $\vec{g}$ is the gravity vector. The buoyancy model accepts arbitrary temperature-dependent functions for density. The `LinearFVMomentumBuoyancy` kernel allows for modeling natural convection.

This term deals only with the force due to the variation in density $\Delta \rho \vec{g}$, with the fluid density being $\rho = \rho_{ref}+\Delta\rho$. Thus, with no extra added terms to the conventional incompressible Navier Stokes equations, the system will solve for the total pressure minus the hydrostatic pressure.

For stability purposes, the value of $\rho_{ref}$ is recommended be set to the midpoint of the estimated maximum and minimum temperature values in the simulation.

For natural convection simulations, it is advisable to compute relevant dimensionless numbers such as the Rayleigh number or the Richardson number to decide on the need for turbulence models, mesh refinement and stability considerations.

!syntax parameters /LinearFVKernels/LinearFVMomentumBuoyancy

!syntax inputs /LinearFVKernels/LinearFVMomentumBuoyancy

!syntax children /LinearFVKernels/LinearFVMomentumBuoyancy
