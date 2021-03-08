# PINSFVMomentumBoussinesq

This object adds a $\epsilon\rho\alpha\vec{g}(T - T_{ref})$ term to the incompressible porous media
Navier Stokes (PINS) momentum equations where $\epsilon$ is the porosity, $\rho$ is the density, $\alpha$
is the thermal expansion coefficient, $\vec{g}$ is the gravity vector, $T$ is the temperature,
and $T_{ref}$ is a reference temperature. The term above introduces the
Boussinesq approximation into the PINS equations, which allows for modeling
natural convection and the effect of buoyancy.

!syntax parameters /FVKernels/PINSFVMomentumBoussinesq

!syntax inputs /FVKernels/PINSFVMomentumBoussinesq

!syntax children /FVKernels/PINSFVMomentumBoussinesq
