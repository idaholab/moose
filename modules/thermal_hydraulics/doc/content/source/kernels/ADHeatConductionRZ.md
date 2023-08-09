# ADHeatConductionRZ

!syntax description /Kernels/ADHeatConductionRZ

The equation term added is the same as for the [HeatConduction.md] kernel,
except that the residual and Jacobian contributions are integrated with cylindrical coordinates, the axis and origin
of which are being defined through input parameters.

!alert warning
This kernel is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most kernels are added automatically by components. This kernel is created by the
[HeatConductionModel.md] which is used in boundary heat structures.

!syntax parameters /Kernels/ADHeatConductionRZ

!syntax inputs /Kernels/ADHeatConductionRZ

!syntax children /Kernels/ADHeatConductionRZ
