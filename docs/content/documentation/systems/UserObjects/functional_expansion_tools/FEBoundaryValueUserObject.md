# FEBoundaryValueUserObject
!syntax description /UserObjects/FEBoundaryValueUserObject

This `UserObject` captures the moments of an FE representing the value at a boundary. It does this via subclassing the templated `FEIntegralBaseUserObject` with `SideIntegralVariableUserObject` as the policy parameter.

`FEIntegralBaseUserObject` also ensures that the provided `Function` is a subclass of `FunctionSeries`.


!syntax parameters /UserObjects/FEBoundaryValueUserObject

!syntax inputs /UserObjects/FEBoundaryValueUserObject

!syntax children /UserObjects/FEBoundaryValueUserObject
