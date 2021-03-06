# INSFVMomentumBoussinesq

This object adds a $\rho\alpha\vec{g}(T - T_{ref})$ term to the incompressible
Navier Stokes (INS) equations where $\rho$ is the density, $\alpha$ is the thermal
expansion coefficient, $\vec{g}$ is the gravity vector, $T$ is the temperature,
and $T_{ref}$ is a reference temperature. The term above introduces the
Boussinesq approximation into the INS equations, which allows for modeling
natural convection.

!syntax parameters /FVKernels/INSFVMomentumBoussinesq

!syntax inputs /FVKernels/INSFVMomentumBoussinesq

!syntax children /FVKernels/INSFVMomentumBoussinesq
