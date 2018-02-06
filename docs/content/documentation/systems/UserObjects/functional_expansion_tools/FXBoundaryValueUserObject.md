# FX Boundary Value UserObject

!syntax description /UserObjects/FXBoundaryValueUserObject

## Description

This `UserObject` captures the moments of an FX representing the value at a boundary. It does this via subclassing the templated `FXIntegralBaseUserObject` with `SideIntegralVariableUserObject` as the policy parameter.

`FXIntegralBaseUserObject` also ensures that the provided `Function` is a subclass of `FunctionSeries`.

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/2D_interface/sub.i block=UserObjects id=input caption=Example use of FXBoundaryValueUserObject

!syntax parameters /UserObjects/FXBoundaryValueUserObject

!syntax inputs /UserObjects/FXBoundaryValueUserObject

!syntax children /UserObjects/FXBoundaryValueUserObject
