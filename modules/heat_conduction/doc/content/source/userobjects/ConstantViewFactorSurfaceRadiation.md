# ConstantViewFactorSurfaceRadiation

## Description

`ConstantViewFactorSurfaceRadiation` inherits from [GrayLambertSurfaceRadiationBase](GrayLambertSurfaceRadiationBase.md).
View factors for `ConstantViewFactorSurfaceRadiation` are provided in the MOOSE input file.

## Example Input syntax

!listing modules/heat_conduction/test/tests/gray_lambert_radiator/gray_lambert_cavity.i
block=UserObjects

!syntax parameters /UserObjects/ConstantViewFactorSurfaceRadiation

!syntax inputs /UserObjects/ConstantViewFactorSurfaceRadiation

!syntax children /UserObjects/ConstantViewFactorSurfaceRadiation
