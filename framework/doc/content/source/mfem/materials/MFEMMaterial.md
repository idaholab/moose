# MFEMMaterial

## Summary

!syntax description /Materials/MFEMMaterial

## Overview

`MFEMMaterial` is the base class for materials defined for MFEM problems. They may be defined on one
or more subdomains (blocks); if no subdomains are provided, the material will be applied on all
subdomains in the mesh.

`MFEMMaterial` is intended to allow the specification of `mfem::Coefficient`,
`mfem::VectorCoefficient`, and `mfem::MatrixCoefficient` objects to add to the MFEM problem in a
manner consistent with the standard MOOSE Materials system.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Materials

!syntax parameters /Materials/MFEMMaterial

!syntax inputs /Materials/MFEMMaterial

!syntax children /Materials/MFEMMaterial
