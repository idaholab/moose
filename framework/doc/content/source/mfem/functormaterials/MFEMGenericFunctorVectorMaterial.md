# MFEMGenericFunctorVectorMaterial

!if! function=hasCapability('mfem')

## Summary

!syntax description /FunctorMaterials/MFEMGenericFunctorVectorMaterial

## Overview

`MFEMGenericFunctorVectorMaterial` defines one or more vector material properties with values
obtained from coefficients on one or more subdomains of the mesh, given by the [!param](/FunctorMaterials/MFEMGenericFunctorVectorMaterial/block) parameter
if provided, or applied to the entire mesh if missing. The vector material properties are named
according to members in the [!param](/FunctorMaterials/MFEMGenericFunctorVectorMaterial/prop_names) parameter, with respective coefficients used to get property
values given by the members of [!param](/FunctorMaterials/MFEMGenericFunctorVectorMaterial/prop_values). The coefficients in [!param](/FunctorMaterials/MFEMGenericFunctorVectorMaterial/prop_names) must be vector-valued.

!syntax parameters /FunctorMaterials/MFEMGenericFunctorVectorMaterial

!syntax inputs /FunctorMaterials/MFEMGenericFunctorVectorMaterial

!syntax children /FunctorMaterials/MFEMGenericFunctorVectorMaterial

!if-end!

!else
!include mfem/mfem_warning.md
