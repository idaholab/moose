# INSFEFluidEnergyDirichletBC

!syntax description /BCs/INSFEFluidEnergyDirichletBC

## Overview

If the [!param](/BCs/INSFEFluidEnergyBC/v_fn) parameter is specified, it is used to compute the boundary fluid
velocity. If not, the domain velocity variables are used. The velocity is used to determine whether the boundary is
an inlet.

This boundary condition is only applied if the boundary is an inlet. The fluid temperature is computed using the
[!param](/BCs/INSFEFluidEnergyDirichletBC/T_fn) parameter function, or the [!param](/BCs/INSFEFluidEnergyDirichletBC/T_scalar)
parameter scalar variable, depending on which is specified. The use of scalar variables is intended for coupling with thermal hydraulics components (in SAM).

!syntax parameters /BCs/INSFEFluidEnergyDirichletBC

!syntax inputs /BCs/INSFEFluidEnergyDirichletBC

!syntax children /BCs/INSFEFluidEnergyDirichletBC
