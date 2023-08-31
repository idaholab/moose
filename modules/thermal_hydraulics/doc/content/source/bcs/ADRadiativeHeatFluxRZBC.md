# ADRadiativeHeatFluxRZBC

!syntax description /BCs/ADRadiativeHeatFluxRZBC

!alert warning
This boundary condition is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HSBoundaryRadiation.md] to place a radiative heat flux boundary condition on a cylindrical heat structure.

!syntax parameters /BCs/ADRadiativeHeatFluxRZBC

!syntax inputs /BCs/ADRadiativeHeatFluxRZBC

!syntax children /BCs/ADRadiativeHeatFluxRZBC
