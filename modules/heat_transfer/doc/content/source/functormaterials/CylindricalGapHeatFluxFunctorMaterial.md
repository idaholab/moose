# CylindricalGapHeatFluxFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes the heat flux
across a cylindrical gap due to conduction and/or radiation, assuming infinitely
long cylindrical surfaces.

This is an appropriate approach to use whenever the mesh may not actually represent
the gap distances between surfaces, and instead the radii of the cylindrical
surfaces are provided by [functors](/Functors/index.md).

The following functor materials are added for the conduction heat flux $q_\text{cond}$,
radiation heat flux $q_\text{rad}$, and total heat flux $q$, with names given by the user:

- [!param](/FunctorMaterials/CylindricalGapHeatFluxFunctorMaterial/conduction_heat_flux_name),
  $q_\text{cond}$, as computed in [utils/HeatTransferModels.md#cylindrical_gap_conduction_heat_flux].
- [!param](/FunctorMaterials/CylindricalGapHeatFluxFunctorMaterial/radiation_heat_flux_name),
  $q_\text{rad}$, as computed in [utils/HeatTransferModels.md#cylindrical_gap_radiation_heat_flux].
- [!param](/FunctorMaterials/CylindricalGapHeatFluxFunctorMaterial/total_heat_flux_name),
  $q = q_\text{cond} + q_\text{rad}$.

!syntax parameters /FunctorMaterials/CylindricalGapHeatFluxFunctorMaterial

!syntax inputs /FunctorMaterials/CylindricalGapHeatFluxFunctorMaterial

!syntax children /FunctorMaterials/CylindricalGapHeatFluxFunctorMaterial
