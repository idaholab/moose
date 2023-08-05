# ADExternalAppConvectionHeatTransferRZBC

!syntax description /BCs/ADExternalAppConvectionHeatTransferRZBC

!alert warning
This boundary condition is meant to be used in XY coordinates that are interpreted as general cylindrical coordinates.
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general RZ-coordinate objects.
Stay tuned!

!alert note
In THM, most boundary conditions are added automatically by components. This boundary condition is created by the
[HSBoundaryExternalAppConvection.md] to place a convective heat flux boundary condition on a cylindrical heat structure with the
temperature and heat transfer coefficients provided by an external application.

!syntax parameters /BCs/ADExternalAppConvectionHeatTransferRZBC

!syntax inputs /BCs/ADExternalAppConvectionHeatTransferRZBC

!syntax children /BCs/ADExternalAppConvectionHeatTransferRZBC
