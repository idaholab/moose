# INSFEFluidEnergyBC

!syntax description /BCs/INSFEFluidEnergyBC

This boundary condition can be used for porous media flow using the [!param](/BCs/INSFEFluidEnergyBC/porosity) parameter
to define the porosity.

## Overview

This boundary condition can be used for porous media flow using the [!param](/BCs/INSFEFluidEnergyBC/porosity_elem) parameter
to define the porosity.

If the [!param](/BCs/INSFEFluidEnergyBC/v_fn) parameter is specified, it is used to compute the boundary fluid
velocity. If not, the domain velocity variables are used.
This kind of setup makes this boundary condition flexible to handle both specified velocity and specified pressure (thus velocity is part of the solutions) situations.
This boundary condition is reversible. If the velocity is outgoing from the boundary,
then the temperature considered for the heat flux is computed using the [!param](/BCs/INSFEFluidEnergyBC/temperature)
parameter variable.
If the velocity is such that the fluid enters the boundary, the fluid temperature is computed using the
[!param](/BCs/INSFEFluidEnergyBC/T_fn) parameter function, or the [!param](/BCs/INSFEFluidEnergyBC/T_branch)
parameter scalar variable, depending on which is specified.
The use of scalar variables is intended for coupling with thermal hydraulics components (in SAM).

!syntax parameters /BCs/INSFEFluidEnergyBC

!syntax inputs /BCs/INSFEFluidEnergyBC

!syntax children /BCs/INSFEFluidEnergyBC
