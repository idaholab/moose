# INSFVTKEDWallFunction

!syntax description /FVBCs/INSFVTKEDWallFunction

This boundary condtion should only be used if no wall treatment is added.
Implements wall boundary conditions for the turbulent kinetic energy dissipation rate.
A separate treatment is used for the laminar and logarithmic layers.
A linear blending functions is used to interpolate between both layers.

!syntax parameters /FVBCs/INSFVTKEDWallFunction

!syntax inputs /FVBCs/INSFVTKEDWallFunction

!syntax children /FVBCs/INSFVTKEDWallFunction
