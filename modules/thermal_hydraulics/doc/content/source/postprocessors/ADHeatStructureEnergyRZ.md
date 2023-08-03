# ADHeatStructureEnergyRZ

!syntax description /Postprocessors/ADHeatStructureEnergyRZ

The energy stored in these heat structures is calculated as:

!equation
E = \int_\Omega \rho c_p (T - T_{ref}) n_{units} r d\Omega

where $E$ is the stored energy, $\rho$ the medium density, $c_p$ the medium specific heat capacity,
$T$ the medium temperature, $T_{ref}$ the reference temperature (reference for the enthalpy)
and $n_{units}$ the number of units this heat structure represents and $r$ the local circumference.

This volumetric integral is computed numerically using the [local quadrature](syntax/Executioner/Quadrature/index.md).

!alert warning
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general 2D objects used in general RZ coordinates.
Stay tuned!

!syntax parameters /Postprocessors/ADHeatStructureEnergyRZ

!syntax inputs /Postprocessors/ADHeatStructureEnergyRZ

!syntax children /Postprocessors/ADHeatStructureEnergyRZ
