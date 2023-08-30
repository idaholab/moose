# HeatStructure2DCouplerRZBC

!syntax description /BCs/HeatStructure2DCouplerRZBC

!alert warning
This boundary condition is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HeatStructure2DCoupler.md] to couple the temperature variable on both sides of the boundaries between the heat structure
components. It is added once for each heat structure involved.

!syntax parameters /BCs/HeatStructure2DCouplerRZBC

!syntax inputs /BCs/HeatStructure2DCouplerRZBC

!syntax children /BCs/HeatStructure2DCouplerRZBC
