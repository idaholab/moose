# LinearFVMomentumBoussinesq

This kernel adds the contributions of the Boussinesq buoyancy treatment for density through a force/source term to the right hand side of the momentum equation system for the finite volume SIMPLE segregated solver [SIMPLE.md]. The Boussinesq buoyancy treatment is applicable for low changes in density, and assumes constant density value in all other equation terms.

This term is described by $-\rho_{ref}\alpha\vec{g}(T - T_{ref})$ present in the momentum equation conservation when describing an incompressible fluid, where $\rho_{ref}$ is the reference density, $\alpha$ is the thermal expansion coefficient, $\vec{g}$ is the gravity vector, $T$ is the temperature, and $T_{ref}$ is a reference temperature. The Boussinesq buoyancy model assumes the changes in density as a function of temperature are linear and relevant only in the buoyant force term of the equation system. The Boussinesq kernel allows for modeling natural convection.

This term deals only with the force due to the variation in density $\Delta \rho \vec{g}$, with the fluid density being $\rho = \rho_{ref}+\Delta\rho$. Thus, with no extra added terms to the conventional incompressible Navier Stokes equations, the system will solve for the total pressure minus the hydrostatic pressure.
For natural convection simulations, it is advisable to compute relevant dimensionless numbers such as the Rayleigh number or the Richardson number to decide on the need for turbulence models, mesh refinement and stability considerations.

!syntax parameters /LinearFVKernels/LinearFVMomentumBoussinesq

!syntax inputs /LinearFVKernels/LinearFVMomentumBoussinesq

!syntax children /LinearFVKernels/LinearFVMomentumBoussinesq
