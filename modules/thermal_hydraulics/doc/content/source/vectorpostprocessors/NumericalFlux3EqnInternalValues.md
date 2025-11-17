# NumericalFlux3EqnInternalValues

This vector post-processor computes the mass, momentum, and energy numerical fluxes for
a [FlowChannel1Phase.md] at the internal sides. The following vectors are created:

- `mass_flux`
- `momentum_flux`
- `energy_flux`

!alert note
For a given pair of adjacent elements, the numerical flux can be different on each
side, due to non-conservative terms. In particular, if the cross-sectional area
differs between the elements, a non-conservative flux appears. The approach taken
here is to perform an arithmetic average of the fluxes on each side.

!syntax parameters /VectorPostprocessors/NumericalFlux3EqnInternalValues

!syntax inputs /VectorPostprocessors/NumericalFlux3EqnInternalValues

!syntax children /VectorPostprocessors/NumericalFlux3EqnInternalValues
