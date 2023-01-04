# FVFunctorConvectiveHeatFluxBC

!syntax description /FVBCs/FVFunctorConvectiveHeatFluxBC

# Description

This boundary condition computes convective heat flux $q'' = h \cdot (T - T_{bulk})$, where $h$ is the convective heat transfer coefficient,
$T$ is the temperature, and $T_{bulk}$ is the far-field temperature. Both $h$ and $T_{bulk}$ are functors, which enables various spatial, variable and other dependences.
The domain of the variable can be specified as either a fluid or a solid using the `is_solid` parameter. For a solid domain, the equation above is applied.
For a fluid domain, the negative of the heat flux is applied. This allows for easier implementation of a double Robin
boundary condition.

Similar objects:
- [CoupledConvectiveHeatFluxBC](CoupledConvectiveHeatFluxBC.md) for a similar boundary condition coupled to variables, for finite elements
- [FunctorThermalResistanceBC](FunctorThermalResistanceBC.md) for a combined conduction, convection, and radiative boundary condition, with a constant outside ambient temperature

# Example syntax

In this example, the `FVFunctorConvectiveHeatFluxBC` boundary condition forms a convective boundary condition between
the fluid and the solid.

!listing fv_functor_convective_heat_flux/fv_functor_convective_heat_flux.i block=Mesh FVBCs

!syntax parameters /FVBCs/FVFunctorConvectiveHeatFluxBC

!syntax inputs /FVBCs/FVFunctorConvectiveHeatFluxBC

!syntax children /FVBCs/FVFunctorConvectiveHeatFluxBC
