# INSFEFluidMassBC

!syntax description /BCs/INSFEFluidMassBC

This boundary condition can be used for porous media flow using the [!param](/BCs/INSFEFluidEnergyBC/porosity) parameter
to define the porosity.
This boundary condition can describe both an inlet and an outlet.

## Overview

If either the [!param](/BCs/INSFEFluidMassBC/v_fn) or [!param](/BCs/INSFEFluidMassBC/v_pps) parameters are specified, they are used to compute the boundary fluid velocity. If not, the domain velocity variables are used.
The mass flux is computed as

!equation
\rho v_{bc}

where $\rho$ is the local fluid density and $v_{bc}$ is the boundary fluid velocity. The mass flux times the test
function is the contribution of this boundary condition to the residual.

!syntax parameters /BCs/INSFEFluidMassBC

!syntax inputs /BCs/INSFEFluidMassBC

!syntax children /BCs/INSFEFluidMassBC
