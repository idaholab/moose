# ElementAverageFunctorPostprocessor

`ElementAverageFunctorPostprocessor` computes the average of a functor over elements:

!equation
\bar{u} = \dfrac{\int_{\Omega} u dV}{\int_{\Omega} dV}

where $u$ is the functor. The user can control the discretization of the integral by
specifying [!param](/Postprocessors/ElementAverageFunctorPostprocessor/evaluation_type), with the default being over quadrature points (`QUADRATURE_POINT`). The other option is `CELL_AVERAGE` which is a natural fit for simulations primarily leveraging the cell-centered finite volume method. This average may be restricted to a subset of blocks.

## Example Input Syntax

In this example, `ElementAverageFunctorPostprocessor` is used to compute the domain average of a linear finite
volume variable `u`.

!listing test/tests/kokkos/fe-fv-interplay/aux-read-from-fv.i block=Postprocessors

!syntax parameters /Postprocessors/ElementAverageFunctorPostprocessor

!syntax inputs /Postprocessors/ElementAverageFunctorPostprocessor

!syntax children /Postprocessors/ElementAverageFunctorPostprocessor
