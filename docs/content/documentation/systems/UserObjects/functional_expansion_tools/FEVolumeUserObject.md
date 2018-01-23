# FEVolumeUserObject
!syntax description /UserObjects/FEVolumeUserObject

This `UserObject` captures the moments of an FE representing the field value over a volume. It does this via subclassing the templated `FEIntegralBaseUserObject` with `ElementIntegralVariableUserObject` as the policy parameter.

`FEIntegralBaseUserObject` also ensures that the provided `Function` is a subclass of `FunctionSeries`.


!syntax parameters /UserObjects/FEVolumeUserObject

!syntax inputs /UserObjects/FEVolumeUserObject

!syntax children /UserObjects/FEVolumeUserObject
