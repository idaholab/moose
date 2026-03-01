# MFEMNodalProjector

!if! function=hasCapability('mfem')

Auxiliary class for extracting nodal positions from MFEM FESpaces, and for setting DoFs on MFEM GridFunctions
via projection given a vector of values evaluated at these nodes.

End users should not need to interact with `MFEMNodalProjector` directly. `MFEMNodalProjector` is 
used by MFEM transfers that set values of an MFEMGridFunction, such as 
[MultiAppMFEMShapeEvaluationTransfer](MultiAppMFEMShapeEvaluationTransfer.md).

!if-end!

!else
!include mfem/mfem_warning.md
