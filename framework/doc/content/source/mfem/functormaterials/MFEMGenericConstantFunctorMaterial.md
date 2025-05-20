# MFEMGenericConstantFunctorMaterial

!if! function=hasCapability('mfem')

## Summary

!syntax description /FunctorMaterials/MFEMGenericConstantFunctorMaterial

## Overview

`MFEMGenericConstantFunctorMaterial` defines one or more scalar material properties with constant values on
one or more subdomains of the mesh, given by the `blocks` parameter if provided, or applied to the
entire mesh if missing. The scalar material properties are named according to members in the
`prop_names` parameter, with respective (constant) values given by the members of `prop_values`.

## Example Input File Syntax

!listing test/tests/mfem/kernels/heattransfer.i block=Materials

!syntax parameters /FunctorMaterials/MFEMGenericConstantFunctorMaterial

!syntax inputs /FunctorMaterials/MFEMGenericConstantFunctorMaterial

!syntax children /FunctorMaterials/MFEMGenericConstantFunctorMaterial

!if-end!

!else
!include mfem/mfem_warning.md
