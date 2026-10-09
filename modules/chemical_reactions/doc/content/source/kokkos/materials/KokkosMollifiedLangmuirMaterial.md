# KokkosMollifiedLangmuirMaterial

!if! function=hasCapability('kokkos')

This is the Kokkos version of [MollifiedLangmuirMaterial](MollifiedLangmuirMaterial.md). See the original document for details.

## Example Input Syntax

The test input below uses [KokkosLangmuirMaterial](KokkosLangmuirMaterial.md) and is run with this material by setting `Materials/lang_stuff/type=KokkosMollifiedLangmuirMaterial` and [!param](/Materials/KokkosMollifiedLangmuirMaterial/mollifier) from the command line.

!listing chemical_reactions/test/tests/kokkos/kokkos_langmuir_desorption.i start=[lang_stuff] end=[] include-end=true

!syntax parameters /Materials/KokkosMollifiedLangmuirMaterial

!syntax inputs /Materials/KokkosMollifiedLangmuirMaterial

!syntax children /Materials/KokkosMollifiedLangmuirMaterial

!if-end!

!else
!include kokkos/kokkos_warning.md
