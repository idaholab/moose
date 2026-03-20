# CubitBlockInfo

!if! function=hasCapability('mfem')

This class stores information on the elements in a `libMesh`-based
`MooseMesh` object. It keeps track of the different blocks in a mesh,
the type of element contained by each block, the IDs and nodes for
each element, and the coordinates of each node. It can also provide
information on the structure of each element type and how it can be
represented in MFEM.

!if-end!

!else
!include mfem/mfem_warning.md
