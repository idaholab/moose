# MFEMGenericConstantVectorFunctorMaterial

!if! function=hasCapability('mfem')

## Summary

!syntax description /FunctorMaterials/MFEMGenericConstantVectorFunctorMaterial

## Overview

`MFEMGenericConstantVectorFunctorMaterial` defines one or more vector material properties with constant
values on one or more subdomains of the mesh, given by the `blocks` parameter if provided, or
applied to the entire mesh if missing. The vector material properties are named according to members
in the `prop_names` parameter, with constant property values given by the members of `prop_values`.
The number of `prop_values` should be the number of `prop_names` multiplied by number of vector
components, assumed to be the mesh dimension.

!syntax parameters /FunctorMaterials/MFEMGenericConstantVectorFunctorMaterial

!syntax inputs /FunctorMaterials/MFEMGenericConstantVectorFunctorMaterial

!syntax children /FunctorMaterials/MFEMGenericConstantVectorFunctorMaterial

!if-end!

!else
!include mfem/mfem_warning.md
