# INSFEFluidWallMomentumBC

!syntax description /BCs/INSFEFluidWallMomentumBC

This boundary condition can be used for porous media flow using the [!param](/BCs/INSFEFluidEnergyBC/porosity) parameter
to define the porosity.
This boundary condition must be specified for each component of the momentum.

## Overview

The momentum flux is computed as the sum of a viscous term, the wall shear stress, (only for near-unity porosity)

!equation
-(\mu + \mu_t) \nabla_u(comp) \vec{n}(comp)

where $\mu$ is the dynamic viscosity, $\mu_t$ is the turbulent viscosity, $\nabla_u$ is the gradient of
velocity and $\vec{n}$ is the local boundary normal, for which only the $comp$ component is used.

!alert note
This boundary condition can only be used if the momentum equation pressure term is integrated by parts.

!syntax parameters /BCs/INSFEFluidWallMomentumBC

!syntax inputs /BCs/INSFEFluidWallMomentumBC

!syntax children /BCs/INSFEFluidWallMomentumBC
