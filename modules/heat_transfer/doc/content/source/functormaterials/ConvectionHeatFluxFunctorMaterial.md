# (AD)ConvectionHeatFluxFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes a convection heat
flux $q$ between a solid surface and a fluid using [functors](/Functors/index.md):

!equation
q \equiv \mathbf{q}\cdot\mathbf{n}_\text{solid} = h (T_\text{solid} - T_\text{fluid}) \,,

where

- $h$ is the heat transfer coefficient, given by [!param](/FunctorMaterials/ConvectionHeatFluxFunctorMaterial/htc),
- $\mathbf{n}_\text{solid}$ is the outward normal unit vector from the solid surface,
- $T_\text{solid}$ is the solid temperature, given by [!param](/FunctorMaterials/ConvectionHeatFluxFunctorMaterial/T_solid), and
- $T_\text{fluid}$ is the fluid temperature, given by [!param](/FunctorMaterials/ConvectionHeatFluxFunctorMaterial/T_fluid).

The AD version of this class is used to retrieve all of the input functors with AD types.

!syntax parameters /FunctorMaterials/ConvectionHeatFluxFunctorMaterial

!syntax inputs /FunctorMaterials/ConvectionHeatFluxFunctorMaterial

!syntax children /FunctorMaterials/ConvectionHeatFluxFunctorMaterial
