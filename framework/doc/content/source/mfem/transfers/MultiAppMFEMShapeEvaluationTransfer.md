# MultiAppMFEMShapeEvaluationTransfer

!if! function=hasCapability('mfem')

Allows transfers of [MFEM variables](MFEMVariable.md) between MFEM-based applications via local
evaluation of shape functions at target nodal projection points. The variables may be defined on
different meshes and FESpaces of different orders, but must be real, share the same dimensionality,
and both belong to an `MFEMProblem` in their respective applications.

## General Description

`MultiAppMFEMShapeEvaluationTransfer` executes transfers in three steps:

1. Extraction of a vector of node positions in the `ParFiniteElementSpace` of the destination
   `mfem::ParGridFunction`, using [`MFEMNodalProjector`](MFEMNodalProjector.md).
2. Interpolation of the source `mfem::ParGridFunction` at this set of node locations, using
   `mfem::FindPointsGSLIB` to perform the required gather/scatter operations to obtain a set of
   points on the local mesh partition, and perform shape function evaluations at these points.
3. Projection of the evaluated source variable values onto the destination variable nodes, to set
   the destination variable degrees of freedom, using [`MFEMNodalProjector`](MFEMNodalProjector.md).

For transfers between identical variables defined on the same mesh, users are recommended to use
[`MultiAppMFEMCopyTransfer`](MultiAppMFEMCopyTransfer.md) for performance.

## Features Supported

All MFEM to MFEM transfers executed from this class should be able to support:

- transfers of scalar and vector variables
- transfers between variables belonging to different finite element spaces and orders
- transfers between variables defined on submeshes and parent meshes
- arbitrary number of parallel processes for both the source and target application
- transfers between parent and child applications
- transfers between sibling applications (child to child)
- transfers from multiple variables to multiple variables

!syntax parameters /Transfers/MultiAppMFEMShapeEvaluationTransfer

!syntax inputs /Transfers/MultiAppMFEMShapeEvaluationTransfer

!syntax children /Transfers/MultiAppMFEMShapeEvaluationTransfer

!if-end!

!else
!include mfem/mfem_warning.md
