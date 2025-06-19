# NEML2FEInterpolation

This userobject loops through elements to cache shape functions and gradient of shape functions. Variables coupled through this userobject will be interpolated onto their corresponding function spaces.

## Implementation details

Once the shape function caching is done, this object does not actively update the function space information unless the `invalidateFEMContext()` method is called. The `contextUpToDate()` method can be used to check if the current FE context cache is up-to-date. Similarly, the variable interpolations are not actively updated until the `invalidateInterpolations()` method is called.

Several getter methods are provided to access cached data:
- `getValue` for getting the variable value of a MOOSE nonlinear variable
- `getGradient` for getting the gradient of a MOOSE nonlinear variable
- `getPhi` and `getPhiGradient` for getting the value and gradient of a MOOSE variable
- `getDofMap` and `getGlobalDofMap` for getting the local/global dof map associated with a MOOSE nonlinear variable
- ...

## Syntax

!syntax parameters /UserObjects/NEML2FEInterpolation
