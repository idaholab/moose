# SideDiffusiveFluxAverage

!syntax description /Postprocessors/SideDiffusiveFluxAverage

!alert warning
The expression of the diffusive flux in this object is generic, as described, and may differ from the diffusive flux in your specific physics implementation. If so, you may not use this object to compute the diffusive flux.

## Example Input Syntax

!listing test/tests/thewarehouse/test1.i block=Postprocessors/avg_flux_right

!syntax parameters /Postprocessors/SideDiffusiveFluxAverage

!syntax inputs /Postprocessors/SideDiffusiveFluxAverage

!syntax children /Postprocessors/SideDiffusiveFluxAverage
