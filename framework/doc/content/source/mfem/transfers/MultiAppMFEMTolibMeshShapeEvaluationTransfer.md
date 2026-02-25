# MultiAppMFEMTolibMeshShapeEvaluationTransfer

!if! function=hasCapability('mfem')

Allows transfers of [MFEM variables](MFEMVariable.md) from MFEM-based applications to libMesh-based [MooseVariables](MooseVariable.md) via local
evaluation of shape functions at target nodal projection points. This class supports transfers between different meshes, from scalar MFEM GridFunctions to first and second LAGRANGE and constant MONOMIAL libMesh variable types.

For transfers in the opposite direction, from libMesh-based applications to MFEM-based applications, please see [MultiApplibMeshToMFEMShapeEvaluationTransfer](MultiApplibMeshToMFEMShapeEvaluationTransfer.md).

## General Description

`MultiAppMFEMTolibMeshShapeEvaluationTransfer` executes transfers in three steps:

1. Extraction of a vector of node positions or element centroids of the destination libMesh solution
   variable (similar to the method used in
   [`MultiAppGeneralFieldTransfer`](MultiAppGeneralFieldTransfer.md)).
2. Interpolation of the source `mfem::ParGridFunction` at this set of node locations, using
   `mfem::FindPointsGSLIB` to perform the required gather/scatter operations to obtain a set of
   points on the local mesh partition, and perform shape function evaluations at these points.
3. Projection of the evaluated source variable values onto the destination libMesh variable nodes,
   to set the destination variable degrees of freedom.

## Features Supported

All MFEM to libMesh transfers executed from this class should be able to support:

- transfers to lowest order scalar LAGRANGE and MONOMIAL variables
- transfers to second order scalar LAGRANGE variables 
- transfers between variables belonging to dissimilar meshes
- arbitrary number of parallel processes for both the source and target application
- transfers between parent and child applications
- transfers between sibling applications (child to child)
- transfers from multiple variables to multiple variables

!syntax parameters /Transfers/MultiAppMFEMTolibMeshShapeEvaluationTransfer

!syntax inputs /Transfers/MultiAppMFEMTolibMeshShapeEvaluationTransfer

!syntax children /Transfers/MultiAppMFEMTolibMeshShapeEvaluationTransfer

!if-end!

!else
!include mfem/mfem_warning.md
