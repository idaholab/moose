# MFEMGenericFunctorMaterial

!if! function=hasCapability('mfem')

## Summary

!syntax description /FunctorMaterials/MFEMGenericFunctorMaterial

## Overview

`MFEMGenericFunctorMaterial` defines one or more scalar material properties with values obtained from a coefficient on
one or more subdomains of the mesh, given by the [!param](/FunctorMaterials/MFEMGenericFunctorMaterial/block) parameter if provided, or applied to the
entire mesh if missing. The scalar material properties are named according to members in the
[!param](/FunctorMaterials/MFEMGenericFunctorMaterial/prop_names) parameter, with respective coefficients used to get property values given by the members of [!param](/FunctorMaterials/MFEMGenericFunctorMaterial/prop_values).

!syntax parameters /FunctorMaterials/MFEMGenericFunctorMaterial

!syntax inputs /FunctorMaterials/MFEMGenericFunctorMaterial

!syntax children /FunctorMaterials/MFEMGenericFunctorMaterial

!if-end!

!else
!include mfem/mfem_warning.md
