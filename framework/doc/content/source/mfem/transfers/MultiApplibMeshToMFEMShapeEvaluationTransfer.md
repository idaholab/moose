# MultiApplibMeshToMFEMShapeEvaluationTransfer

!if! function=hasCapability('mfem')

Allows transfers of libMesh-based [MooseVariables](MooseVariable.md) to 
[MFEM variables](MFEMVariable.md),
via local evaluation of shape functions at target nodal projection points. This class supports
transfers between different meshes, from first and second LAGRANGE and constant MONOMIAL libMesh
variable types to scalar MFEM GridFunctions defined on H1 and L2 conforming finite element spaces.

For transfers in the opposite direction, from MFEM-based applications to libMesh-based applications, please see [MultiAppMFEMTolibMeshShapeEvaluationTransfer](MultiAppMFEMTolibMeshShapeEvaluationTransfer.md).

## General Description

`MultiApplibMeshToMFEMShapeEvaluationTransfer` executes transfers in three steps:

1. Extraction of a vector of node positions in the `ParFiniteElementSpace` of the destination
   `mfem::ParGridFunction`, using [`MFEMNodalProjector`](MFEMNodalProjector.md).
2. Interpolation of the source libMesh-based `MooseVariable` at this set of node locations. Required
   gather/scatter operations are performed similarly to those in
   [`MultiAppGeneralFieldTransfer`](MultiAppGeneralFieldTransfer.md), with interpolation analagous
   to [`MultiAppShapeEvaluationTransfer`](MultiAppShapeEvaluationTransfer.md).
3. Projection of the evaluated source variable values onto the destination variable nodes, to set
   the destination variable degrees of freedom, using [`MFEMNodalProjector`](MFEMNodalProjector.md).

## Features Supported

All libMesh to MFEM transfers executed from this class should be able to support:

- transfers from lowest order scalar LAGRANGE and MONOMIAL variables
- transfers from second order scalar LAGRANGE variables 
- transfers between variables belonging to dissimilar meshes
- arbitrary number of parallel processes for both the source and target application
- transfers between parent and child applications
- transfers between sibling applications (child to child)
- transfers from multiple variables to multiple variables

!syntax parameters /Transfers/MultiApplibMeshToMFEMShapeEvaluationTransfer

!syntax inputs /Transfers/MultiApplibMeshToMFEMShapeEvaluationTransfer

!syntax children /Transfers/MultiApplibMeshToMFEMShapeEvaluationTransfer

!if-end!

!else
!include mfem/mfem_warning.md
