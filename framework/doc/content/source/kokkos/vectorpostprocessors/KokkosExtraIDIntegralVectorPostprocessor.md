# KokkosExtraIDIntegralVectorPostprocessor

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ExtraIDIntegralVectorPostprocessor](ExtraIDIntegralVectorPostprocessor.md). See the original document for details.

!alert note
This vector postprocessor provides the capability to choose the calculation mode between `atomic` and `reduction` through the [!param](/VectorPostprocessors/KokkosExtraIDIntegralVectorPostprocessor/calculation_mode) parameter.
When accumulating variable or material property values for each extra element ID, the former uses atomic addition and the latter uses reduction.
When there are only a handful number of unique extra element IDs, the reduction mode can be better for performance, while the atomic mode can be beneficial for many extra element IDs.
See [this page](syntax/KokkosUserObjects/index.md#reducer_atomic) for further discussions.

## Example Input Syntax

!listing test/tests/kokkos/vectorpostprocessors/extra_id_integral/kokkos_extra_id_vpp.i start=[integral] end=[] include-end=true

!syntax parameters /VectorPostprocessors/KokkosExtraIDIntegralVectorPostprocessor

!syntax inputs /VectorPostprocessors/KokkosExtraIDIntegralVectorPostprocessor

!syntax children /VectorPostprocessors/KokkosExtraIDIntegralVectorPostprocessor

!if-end!

!else
!include kokkos/kokkos_warning.md
