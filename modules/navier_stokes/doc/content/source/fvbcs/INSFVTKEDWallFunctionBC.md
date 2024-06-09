# INSFVTKEDWallFunctionBC

This boundary condition should only be used if no wall treatment is added.
Implements wall boundary conditions for the turbulent kinetic energy dissipation rate.
A separate treatment is used for the laminar and logarithmic layers.
A linear blending functions is used to interpolate between both layers.

!syntax parameters /FVBCs/INSFVTKEDWallFunctionBC

!syntax inputs /FVBCs/INSFVTKEDWallFunctionBC

!syntax children /FVBCs/INSFVTKEDWallFunctionBC

!tag name=INSFVTKEDWallFunctionBC pairs=module:navier_stokes system:fvbcs
