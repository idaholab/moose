# KokkosConvectiveHeatTransferSideIntegral

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ConvectiveHeatTransferSideIntegral](ConvectiveHeatTransferSideIntegral.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_convective_ht_side_integral.i start=[Qw3] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosConvectiveHeatTransferSideIntegral

!syntax inputs /Postprocessors/KokkosConvectiveHeatTransferSideIntegral

!syntax children /Postprocessors/KokkosConvectiveHeatTransferSideIntegral

!if-end!

!else
!include kokkos/kokkos_warning.md
