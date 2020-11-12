# INSFVPenaltyFreeSlipBC

This object implements a free slip boundary condition. It must be applied to
each velocity component using the `momentum_component = x, y, z` input file
parameter. The exactness of the enforcement is determined by the order of the
`penalty` input file parameter. The default value for `penalty` is `1e6`. The
higher the `penalty` value, the more exact the enforcement of the free slip
condition, but also the more ill-conditioned the resulting system of equations.

!syntax parameters /FVBCs/INSFVPenaltyFreeSlipBC

!syntax inputs /FVBCs/INSFVPenaltyFreeSlipBC

!syntax children /FVBCs/INSFVPenaltyFreeSlipBC
