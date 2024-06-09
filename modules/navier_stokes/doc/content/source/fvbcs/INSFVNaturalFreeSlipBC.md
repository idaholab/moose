# INSFVNaturalFreeSlipBC

This object implements a free slip boundary condition. It should be applied to
each velocity component. This BC operates very simply by setting the total
momentum boundary flux, e.g. the sum of advective and viscous fluxes, to zero.

!syntax parameters /FVBCs/INSFVNaturalFreeSlipBC

!syntax inputs /FVBCs/INSFVNaturalFreeSlipBC

!syntax children /FVBCs/INSFVNaturalFreeSlipBC

!tag name=INSFVNaturalFreeSlipBC pairs=module:navier_stokes system:fvbcs
