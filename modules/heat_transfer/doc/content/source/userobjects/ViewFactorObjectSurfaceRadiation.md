# ViewFactorObjectSurfaceRadiation

## Description

`ViewFactorObjectSurfaceRadiation` inherits from `GrayLambertSurfaceRadiationBase` and allows automatic computation of view factors.
View factors for `ViewFactorObjectSurfaceRadiation` are provided by a `ViewFactorBase` object, e.g. [UnobstructedPlanarViewFactor](UnobstructedPlanarViewFactor.md).

## Example Input syntax

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/gray_lambert_cavity_automatic_vf.i
block=UserObjects

!syntax parameters /UserObjects/ViewFactorObjectSurfaceRadiation

!syntax inputs /UserObjects/ViewFactorObjectSurfaceRadiation

!syntax children /UserObjects/ViewFactorObjectSurfaceRadiation
