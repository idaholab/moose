# FX Boundary Flux UserObject

!syntax description /UserObjects/FXBoundaryFluxUserObject

## Description

This `UserObject` captures the moments of an FX representing the flux at a boundary. It does this via subclassing the templated `FXIntegralBaseUserObject` with `SideIntegralVariableUserObject` as the policy parameter. It also requires a diffusivity parameter, which can either be a constant value or the name of a material property.

`FXIntegralBaseUserObject` also ensures that the provided `Function` is a subclass of `FunctionSeries`.

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/2D_interface/main.i block=UserObjects id=input caption=Example use of FXBoundaryFluxUserObject

!syntax parameters /UserObjects/FXBoundaryFluxUserObject

!syntax inputs /UserObjects/FXBoundaryFluxUserObject

!syntax children /UserObjects/FXBoundaryFluxUserObject
