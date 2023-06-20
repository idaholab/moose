# ADConvectionHeatFluxMaterial

!syntax description /Materials/ADConvectionHeatFluxMaterial

The heat flux $q$ is computed as:

!equation
q = \kappa h_{wall} (T_{wall} - T)

with $\kappa$ the fluid phase wall contact fraction, $h_{wall}$ the wall heat transfer coefficient,
$T_{wall}$ the wall temperature, and $T$ the fluid temperature.

!syntax parameters /Materials/ADConvectionHeatFluxMaterial

!syntax inputs /Materials/ADConvectionHeatFluxMaterial

!syntax children /Materials/ADConvectionHeatFluxMaterial
