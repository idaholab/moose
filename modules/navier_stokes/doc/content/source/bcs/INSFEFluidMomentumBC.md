# INSFEFluidMomentumBC

!syntax description /BCs/INSFEFluidMomentumBC

This boundary condition can be used for porous media flow using the [!param](/BCs/INSFEFluidEnergyBC/porosity) parameter
to define the porosity.
This boundary condition can describe both an inlet and an outlet.
This boundary condition must be specified for each component of the momentum.

## Overview

Either the boundary pressure or the boundary velocity should be specified. The boundary pressure can be specified
to be a scalar variable using the [!param](/BCs/INSFEFluidMomentumBC/p_branch) parameter. This is intended for coupling
with thermal hydraulics components (in SAM).


The momentum flux is computed as the sum of a viscous term (only for near-unity porosity)

!equation
-(\mu + \mu_t) \nabla_u \cdot \vec{n}

where $\mu$ is the dynamic viscosity, $\mu_t$ is the turbulent viscosity, $\nabla_u$ is the gradient of
velocity and $\vec{n}$ is the local boundary normal.

a pressure term (only if integrating pressure by parts)

!equation
\epsilon p (e_u \cdot \vec{n})

where $\epsilon$ is the local porosity, $p$ the boundary pressure, and $e_u \cdot \vec{n}$ the component of the local
boundary normal.
and a convection term (only if using the conservative form)

!equation
\rho u v_{bc} / \epsilon

where $\rho$ is the local fluid density, $u$ the velocity component and $v_{bc}$ is the boundary fluid velocity times the local normal.

!alert note
The [!param](/BCs/INSFEFluidMomentumBC/p_int_by_parts) and [!param](/BCs/INSFEFluidMomentumBC/conservative_form)
parameter must be consistent with the formulation of the equations in the kernels.

!syntax parameters /BCs/INSFEFluidMomentumBC

!syntax inputs /BCs/INSFEFluidMomentumBC

!syntax children /BCs/INSFEFluidMomentumBC
