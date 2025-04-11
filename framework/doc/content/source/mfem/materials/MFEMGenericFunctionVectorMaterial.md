# MFEMGenericFunctionVectorMaterial

## Summary

!syntax description /Materials/MFEMGenericFunctionVectorMaterial

## Overview

`MFEMGenericFunctionVectorMaterial` defines one or more vector material properties with values
obtained from MOOSE functions on one or more subdomains of the mesh, given by the `blocks` parameter
if provided, or applied to the entire mesh if missing. The vector material properties are named
according to members in the `prop_names` parameter, with respective functions used to get property
values given by the members of `prop_values`. The functions in `prop_names` must be vector-valued
functions.

!syntax parameters /Materials/MFEMGenericFunctionVectorMaterial

!syntax inputs /Materials/MFEMGenericFunctionVectorMaterial

!syntax children /Materials/MFEMGenericFunctionVectorMaterial
