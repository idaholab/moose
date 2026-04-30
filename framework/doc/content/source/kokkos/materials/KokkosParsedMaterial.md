# KokkosParsedMaterial

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ParsedMaterial](ParsedMaterial.md). See the original document for details.

!alert note
The Kokkos version does not support functors yet. Instead, postprocessors, variables, material properties, and functions can be specified for the use in the parsed expression through separate parameters.

## Example Input Syntax

!listing test/tests/kokkos/materials/parsed/kokkos_parsed_material.i start=[mat] end=[] include-end=true

!syntax parameters /Materials/KokkosParsedMaterial

!syntax inputs /Materials/KokkosParsedMaterial

!syntax children /Materials/KokkosParsedMaterial

!if-end!

!else
!include kokkos/kokkos_warning.md
