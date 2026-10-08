# KokkosLangmuirMaterial

!if! function=hasCapability('kokkos')

This is the Kokkos version of [LangmuirMaterial](LangmuirMaterial.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_langmuir_desorption.i start=[lang_stuff] end=[] include-end=true

!syntax parameters /Materials/KokkosLangmuirMaterial

!syntax inputs /Materials/KokkosLangmuirMaterial

!syntax children /Materials/KokkosLangmuirMaterial

!if-end!

!else
!include kokkos/kokkos_warning.md
