# SurfaceRadiationVectorPostprocessor

!syntax description /VectorPostprocessors/SurfaceRadiationVectorPostprocessor

## Description

SurfaceRadiationVectorPostprocessor outputs
a selection of temperature, heat_flux_density, radiosity, and emissivity
from a gray, diffuse view factor calculation for all participating boundaries.
The information is obtained from a `GrayLambertSurfaceRadiationBase` or derived
object.


!listing modules/heat_conduction/test/tests/gray_lambert_radiator/gray_lambert_cavity.i start=[./lambert_vpp] end=[../] include-end=true

!syntax parameters /VectorPostprocessors/SurfaceRadiationVectorPostprocessor

!syntax inputs /VectorPostprocessors/SurfaceRadiationVectorPostprocessor

!syntax children /VectorPostprocessors/SurfaceRadiationVectorPostprocessor

!bibtex bibliography
