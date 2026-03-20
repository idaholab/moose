# buildMFEMMesh

!if! function=hasCapability('mfem')

The `buildMFEMMesh` function is used to obtain an `mfem::ParMesh`
object from a `MooseMesh` object. It is called by the constructor for
an `MFEMProblem`. It takes two additional arguments:

- `fallback`: If true and certain element types which can't be
  represented in MFEM are encountered then the function will produce a
  warning and fall-back to using the closest-available
  approximation. Otherwise it will produce an error.
- `first_order`: If true then all elements will be forced to be
  first-order in MFEM.

## Overview

When an `MFEMMesh` is passed as the argument to `buildMFEMMesh` then
the underlying `mfem::ParMesh` object it contains will be
returned. Otherwise, the function will attempt to create an
`mfem::ParMesh` that corresponds to the `libMesh`-based `MooseMesh`
object. It currently supports the following element types:

- NODEELEM
- EDGE2
- EDGE3
- EDGE4
- TRI3
- TRI6
- QUAD4
- QUAD8
- QUAD9
- TET4
- TET10
- HEX8
- HEX20
- HEX27
- PRISM6
- PRISM15
- PRISM18
- PYRAMID5

Additionally, if the argument `fallback` is true, then the following
elements will be approximated as described below:

- TRI7 → TRI6
- TET14 → TET10
- PRISM20 → PRISM18
- PRISM21 → PRISM18

Higher-order pyramids are not currently supported, due to [a bug in
MFEM](https://github.com/mfem/mfem/issues/5256). Support for these
element types (including falling-back from PYRAMID18 →
PYRAMID14) will be added once that bug is fixed.

If the argument `first_order` is true then all elements will be
converted to their corresponding first-order types. For example, a
HEX20 would be represented as a HEX8.  This is useful in cases where
second-order elements were used for higher-order basis functions in
MOOSE, even though the shape of the elements could have been
represented with first-order. Because the order of basis functions for
representing variables and for describing mesh geometry are
independent in MFEM, it then becomes unnecessary (and a waste of
memory) to use a higher-order `mfem::ParMesh` object. When
`first_order` is true, higher-order pyramids will be successfully
converted to first-order pyramids, without an error. It also becomes
unnecessary to use the "fall-back" element types described above.

!if-end!

!else
!include mfem/mfem_warning.md
