# SideDiffusiveFluxAverage

!syntax description /Postprocessors/SideDiffusiveFluxAverage

## Object Description

This object computes the quantity

!equation
- \frac{1}{A} \int_{d\Omega} D \nabla u \cdot \mathbf{n}

where:

- $d\Omega$ represents the boundary,
- $D$ represents the diffusivity,
- $u$ represents the variable used in the calculation,
- $\mathbf{n}$ represents the outward-facing normal vector at the boundary, and
- $A$ represents the area of the selected boundary across which the average is being taken.

!alert warning
The expression of the diffusive flux in this object is generic, as described, and may differ from the diffusive flux in your specific physics implementation. If so, you may not use this object to compute the diffusive flux.

## Example Input Syntax

!listing test/tests/thewarehouse/test1.i block=Postprocessors/avg_flux_right

!syntax parameters /Postprocessors/SideDiffusiveFluxAverage

!syntax inputs /Postprocessors/SideDiffusiveFluxAverage

!syntax children /Postprocessors/SideDiffusiveFluxAverage
