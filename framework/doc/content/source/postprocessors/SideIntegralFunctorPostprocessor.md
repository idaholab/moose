# SideIntegralFunctorPostprocessor

!syntax description /Postprocessors/SideIntegralFunctorPostprocessor

The `SideIntegralFunctorPostprocessor` is also an intermediate base class
that should be derived from for any calculation involving
the integral of a functor quantity over a side.

The [!param](/Postprocessors/SideIntegralFunctorPostprocessor/functor_argument) parameter lets
the user select between face integration and element side quadrature point integration. Face integration
is more appropriate for finite volume variables, and quadrature point integration is more appropriate
for finite element variables.

!syntax parameters /Postprocessors/SideIntegralFunctorPostprocessor

!syntax inputs /Postprocessors/SideIntegralFunctorPostprocessor

!syntax children /Postprocessors/SideIntegralFunctorPostprocessor
