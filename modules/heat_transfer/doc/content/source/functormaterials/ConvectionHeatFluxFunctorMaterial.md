# (AD)ConvectionHeatFluxFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes a convection heat
flux $q$ between a solid surface and a fluid using [functors](/Functors/index.md):

!equation
q = -\gamma\mathbf{q}\cdot\mathbf{n}_\text{solid} = \gamma h (T_\text{fluid} - T_\text{solid}) \,,

where

- $\gamma$ is equal to 1 if $q$ is requested to correspond to the incoming heat flux
  to the solid and -1 if $q$ is requested to correspond to the incoming heat flux to the fluid, determined
  by [!param](/FunctorMaterials/ConvectionHeatFluxFunctorMaterial/to_solid),
- $h$ is the heat transfer coefficient, given by [!param](/FunctorMaterials/ConvectionHeatFluxFunctorMaterial/htc),
- $\mathbf{n}_\text{solid}$ is the outward normal unit vector from the solid surface,
- $T_\text{fluid}$ is the fluid temperature, given by [!param](/FunctorMaterials/ConvectionHeatFluxFunctorMaterial/T_fluid), and
- $T_\text{solid}$ is the solid temperature, given by [!param](/FunctorMaterials/ConvectionHeatFluxFunctorMaterial/T_solid).

The AD version of this class is used to retrieve all of the input functors with AD types.

!syntax parameters /FunctorMaterials/ConvectionHeatFluxFunctorMaterial

!syntax inputs /FunctorMaterials/ConvectionHeatFluxFunctorMaterial

!syntax children /FunctorMaterials/ConvectionHeatFluxFunctorMaterial
