# INSFVInletVelocityBC

This object simply wraps [`FVFunctionDirichletBC`](FVFunctionDirichletBC.md), so a
required parameter is [!param](/FVBCs/INSFVInletVelocityBC/functor) describing the velocity along an inlet
boundary. The [!param](/FVBCs/INSFVInletVelocityBC/variable) parameter should correspond to a velocity component
variables. If applying `INSFVInletVelocity` for any velocity component on a given
[!param](/FVBCs/INSFVInletVelocityBC/boundary), then an `INSFVInletVelocity` should be specified for every velocity
component on that `boundary`. A `FVBC` for pressure should not be applied on the
same `boundary`. Instead a value for the pressure at the inlet will be extrapolated
from the interior.

!syntax parameters /FVBCs/INSFVInletVelocityBC

!syntax inputs /FVBCs/INSFVInletVelocityBC

!syntax children /FVBCs/INSFVInletVelocityBC
