# FVFunctorConvectiveHeatFluxBC

!syntax description /FVBCs/FVFunctorConvectiveHeatFluxBC

# Description

This boundary condition computes convective heat flux $q'' = h \cdot (T - T_{inf})$, where $H$ is convective heat transfer coefficient,
$T$ is the temperature, and $T_{inf}$ is far field temperature. Both $H$ and $T_{inf}$ are coupled as functors.
The domain of the variable can be specified as either a fluid or a solid using $is\_solid$. For a solid domain, the equation above is applied.
For a fluid domain, the negative of the heat flux is applied. This allows for easier implementation of a double Robin
boundary condition.

See [CoupledConvectiveHeatFluxBC](CoupledConvectiveHeatFluxBC.md) for a similar boundary condition coupled to variables, and see [FunctorThermalResistanceBC](FunctorThermalResistanceBC.md) for a combined conduction, convection, and radiative boundary condition
 with functor material properties.

!syntax parameters /FVBCs/FVFunctorConvectiveHeatFluxBC

!syntax inputs /FVBCs/FVFunctorConvectiveHeatFluxBC

!syntax children /FVBCs/FVFunctorConvectiveHeatFluxBC
