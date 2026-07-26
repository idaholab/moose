# GrayLambertRadiationHeatFluxAux

This object retrieves the radiation heat flux $q$ computed by [GrayLambertSurfaceRadiationBase.md] objects.
Because `GrayLambertSurfaceRadiationBase` objects store heat flux by sideset ID, it is
recommended to use `CONSTANT MONOMIAL` variables to avoid ambiguous definition of the heat
flux at nodes that are shared between adjacent sidesets.

!syntax parameters /AuxKernels/GrayLambertRadiationHeatFluxAux

!syntax inputs /AuxKernels/GrayLambertRadiationHeatFluxAux

!syntax children /AuxKernels/GrayLambertRadiationHeatFluxAux
