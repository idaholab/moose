# LibmeshMFEMBlockInfo

!if! function=hasCapability('mfem')

This class stores information on the elements in a `libMesh`-based
`MooseMesh` object, for use when converting it to an MFEM mesh. It
keeps track of the different blocks in a mesh and the type of element
contained by each block. It also provides information on the structure
of each `libMesh` element type and how it can be represented in MFEM,
including any fallback or first-order element types used in its place.

!if-end!

!else
!include mfem/mfem_warning.md
