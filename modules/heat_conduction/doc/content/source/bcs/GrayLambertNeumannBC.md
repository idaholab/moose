# GrayLambertNeumannBC

!syntax description /BCs/GrayLambertNeumannBC

This boundary condition computes the heat flux density resulting from the
radiative heat transfer between surfaces adjacent to this boundary. These
surfaces must be diffuse, gray radiators (Lambert radiators). The heat flux
is computed by the net radiation method described in [!cite](modest2013radiative).
More information is available [here](userobjects/GrayLambertSurfaceRadiation.md).

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/coupled_heat_conduction.i
block=BCs

!syntax parameters /BCs/GrayLambertNeumannBC

!syntax inputs /BCs/GrayLambertNeumannBC

!syntax children /BCs/GrayLambertNeumannBC
