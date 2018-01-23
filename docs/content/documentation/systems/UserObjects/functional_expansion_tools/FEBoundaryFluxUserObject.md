# FEBoundaryFluxUserObject
!syntax description /UserObjects/FEBoundaryFluxUserObject

This `UserObject` captures the moments of an FE representing the flux at a boundary. It does this via subclassing the templated `FEIntegralBaseUserObject` with `SideIntegralVariableUserObject` as the policy parameter. It also requires a diffusivity parameter, which can either be a constant value or the name of a material property.

`FEIntegralBaseUserObject` also ensures that the provided `Function` is a subclass of `FunctionSeries`.


!syntax parameters /UserObjects/FEBoundaryFluxUserObject

!syntax inputs /UserObjects/FEBoundaryFluxUserObject

!syntax children /UserObjects/FEBoundaryFluxUserObject
