# ADHeatStructureHeatSourceRZ

!syntax description /Kernels/ADHeatStructureHeatSourceRZ

The equation term added is the same as for the [ADHeatStructureHeatSource.md] kernel,
except that the residual and Jacobian contributions are integrated with cylindrical coordinates, the axis and origin
of which are being defined through input parameters.

!alert warning
This kernel is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most kernels are added automatically by components. This kernel is created by the
[HeatSourceFromTotalPower.md] component which is used to add heat sources to heat structures.

!syntax parameters /Kernels/ADHeatStructureHeatSourceRZ

!syntax inputs /Kernels/ADHeatStructureHeatSourceRZ

!syntax children /Kernels/ADHeatStructureHeatSourceRZ
