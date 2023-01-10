# DirectionalFluxBC

!syntax description /BCs/DirectionalFluxBC

The flux is only applied on the irradiation-facing side of the surface, i.e when the product of flux vector and (outward facing) normal is negative.

A [SelfShadowSideUserObject](SelfShadowSideUserObject.md) can be supplied to take self shadowing into account.

!listing directional_flux_bc/2d.i block=BCs/flux_v

!syntax parameters /BCs/DirectionalFluxBC

!syntax inputs /BCs/DirectionalFluxBC

!syntax children /BCs/DirectionalFluxBC
