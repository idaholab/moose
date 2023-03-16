# ADElementIntegralMaterialPropertyRZ

This post-processor integrates a material property over a 2D RZ domain. This
class derives from [ADElementIntegralMaterialProperty](ElementIntegralMaterialProperty.md)
and [RZSymmetry.md] and multiplies `ADElementIntegralMaterialProperty::computeQpIntegral()`
by the local circumference to achieve the desired RZ integral.

!syntax parameters /Postprocessors/ADElementIntegralMaterialPropertyRZ

!syntax inputs /Postprocessors/ADElementIntegralMaterialPropertyRZ

!syntax children /Postprocessors/ADElementIntegralMaterialPropertyRZ
