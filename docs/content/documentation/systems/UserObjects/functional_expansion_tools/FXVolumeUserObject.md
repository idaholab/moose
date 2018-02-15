# FX Volume UserObject

!syntax description /UserObjects/FXVolumeUserObject

## Description

This `UserObject` captures the moments of an FX representing the field value over a volume. It does this via subclassing the templated `FXIntegralBaseUserObject` with `ElementIntegralVariableUserObject` as the policy parameter.

`FXIntegralBaseUserObject` also ensures that the provided `Function` is a subclass of `FunctionSeries`.

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/1D_volumetric_Cartesian/main.i block=UserObjects id=input caption=Example use of FXVolumeUserObject

!syntax parameters /UserObjects/FXVolumeUserObject

!syntax inputs /UserObjects/FXVolumeUserObject

!syntax children /UserObjects/FXVolumeUserObject
