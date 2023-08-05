# ConvectionHeatTransferRZBC

!syntax description /BCs/ConvectionHeatTransferRZBC

!alert warning
This boundary condition is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADConvectionHeatTransferRZBC.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /BCs/ConvectionHeatTransferRZBC

!syntax inputs /BCs/ConvectionHeatTransferRZBC

!syntax children /BCs/ConvectionHeatTransferRZBC
