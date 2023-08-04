# ADHeatStructureEnergy3D

!syntax description /Postprocessors/ADHeatStructureEnergy3D

The energy stored in the heat structure is calculated as:

!equation
E = \int_\Omega \rho c_p (T - T_{ref}) d\Omega

where $E$ is the stored energy, $\rho$ the medium density, $c_p$ the medium specific heat capacity,
$T$ the medium temperature and $T_{ref}$ the reference temperature (reference for the enthalpy).
Only a single unit of the heat structure is considered.

This volumetric integral is computed numerically using the [local quadrature](syntax/Executioner/Quadrature/index.md).

!syntax parameters /Postprocessors/ADHeatStructureEnergy3D

!syntax inputs /Postprocessors/ADHeatStructureEnergy3D

!syntax children /Postprocessors/ADHeatStructureEnergy3D
