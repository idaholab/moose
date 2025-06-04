# MFEMFunctorMaterial

!if! function=hasCapability('mfem')

## Summary

!syntax description /FunctorMaterials/MFEMFunctorMaterial

## Overview

`MFEMFunctorMaterial` is the base class for materials defined for MFEM problems. They may be defined on one
or more subdomains (blocks); if no subdomains are provided, the material will be applied on all
subdomains in the mesh.

`MFEMFunctorMaterial` is intended to allow the specification of `mfem::Coefficient`,
`mfem::VectorCoefficient`, and `mfem::MatrixCoefficient` objects to add to the MFEM problem in a
manner consistent with the standard MOOSE Materials system.

## Example Input File Syntax

!listing test/tests/mfem/kernels/gravity.i block=Materials

!syntax parameters /FunctorMaterials/MFEMFunctorMaterial

!syntax inputs /FunctorMaterials/MFEMFunctorMaterial

!syntax children /FunctorMaterials/MFEMFunctorMaterial

!if-end!

!else
!include mfem/mfem_warning.md
