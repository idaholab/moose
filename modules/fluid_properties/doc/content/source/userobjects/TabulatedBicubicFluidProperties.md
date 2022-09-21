# TabulatedBicubicFluidProperties

!syntax description /FluidProperties/TabulatedBicubicFluidProperties

See [TabulatedFluidProperties.md] for more information on how to use this object.

!alert warning
Third order fluid property interpolations are not always monotonous. This is exacerbated by composition when using
alternative variable sets such as (specific volume, specific internal energy). If this arises
and is observed to remove the uniqueness of the numerical solution, lower order interpolation should be considered.

!syntax parameters /FluidProperties/TabulatedBicubicFluidProperties

!syntax inputs /FluidProperties/TabulatedBicubicFluidProperties

!syntax children /FluidProperties/TabulatedBicubicFluidProperties
