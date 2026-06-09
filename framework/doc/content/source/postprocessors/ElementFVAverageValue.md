# ElementFVAverageValue

`ElementFVAverageValue` computes the volume-weighted average of a functor over elements:

!equation
\bar{u} = \dfrac{\sum_e u_e\,V_e}{\sum_e V_e}

where $u_e$ is the functor evaluated at the centroid of element $e$ and $V_e$ is its volume
(multiplied by the coordinate factor to support axisymmetric geometries). Unlike
[ElementAverageValue.md], the functor is evaluated once per element at its centroid rather than at
quadrature points, which is the natural evaluation style for finite volume variables.

This average may be restricted to a subset of blocks. For averaging over boundaries, use
[SideAverageValue.md] instead.

## Example Input Syntax

In this example, `ElementFVAverageValue` is used to compute the domain average of a linear finite
volume variable `u`.

!listing test/tests/kokkos/fe-fv-interplay/aux-read-from-fv.i block=Postprocessors

!syntax parameters /Postprocessors/ElementFVAverageValue

!syntax inputs /Postprocessors/ElementFVAverageValue

!syntax children /Postprocessors/ElementFVAverageValue
