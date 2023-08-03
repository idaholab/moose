# ADHeatStructureEnergy

!syntax description /Postprocessors/ADHeatStructureEnergy

The energy stored in these heat structures is calculated as:

!equation
E = \int_\Omega \rho c_p (T - T_{ref}) n_{units} d d\Omega

where $E$ is the stored energy, $\rho$ the medium density, $c_p$ the medium specific heat capacity,
$T$ the medium temperature, $T_{ref}$ the reference temperature (reference for the enthalpy)
and $n_{units}$ the number of units this heat structure represents, and $d$ is the plate depth, i.e., the
size of uni-dimensional heat structures modeled.

This volumetric integral is computed numerically using the [local quadrature](syntax/Executioner/Quadrature/index.md)

!syntax parameters /Postprocessors/ADHeatStructureEnergy

!syntax inputs /Postprocessors/ADHeatStructureEnergy

!syntax children /Postprocessors/ADHeatStructureEnergy
