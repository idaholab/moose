# MFEMGenericConstantMaterial

## Summary

!syntax description /Materials/MFEMGenericConstantMaterial

## Overview

`MFEMGenericConstantMaterial` defines one or more scalar material properties with constant values on
one or more subdomains of the mesh, given by the `blocks` parameter if provided, or applied to the
entire mesh if missing. The scalar material properties are named according to members in the
`prop_names` parameter, with respective (constant) values given by the members of `prop_values`.

## Example Input File Syntax

!listing test/tests/kernels/heatconduction.i block=Materials

!syntax parameters /Materials/MFEMGenericConstantMaterial

!syntax inputs /Materials/MFEMGenericConstantMaterial

!syntax children /Materials/MFEMGenericConstantMaterial
