# MFEMGenericFunctionMaterial

!if! function=hasCapability('mfem')

## Summary

!syntax description /Materials/MFEMGenericFunctionMaterial

## Overview

`MFEMGenericFunctionMaterial` defines one or more scalar material properties with values obtained from MOOSE functions on
one or more subdomains of the mesh, given by the `blocks` parameter if provided, or applied to the
entire mesh if missing. The scalar material properties are named according to members in the
`prop_names` parameter, with respective functions used to get property values given by the members of `prop_values`.

!syntax parameters /Materials/MFEMGenericFunctionMaterial

!syntax inputs /Materials/MFEMGenericFunctionMaterial

!syntax children /Materials/MFEMGenericFunctionMaterial

!else
!include mfem/mfem_warning.md
