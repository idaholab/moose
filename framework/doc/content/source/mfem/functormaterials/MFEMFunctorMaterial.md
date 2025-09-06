# MFEMFunctorMaterial

!if! function=hasCapability('mfem')

## Summary

Base class for declaration of material properties to add to MFEM problems.

## Overview

`MFEMFunctorMaterial` is the base class for materials defined for MFEM problems. They may be defined on one
or more subdomains (blocks); if no subdomains are provided, the material will be applied on all
subdomains in the mesh.

`MFEMFunctorMaterial` is intended to allow the specification of `mfem::Coefficient`,
`mfem::VectorCoefficient`, and `mfem::MatrixCoefficient` objects to add to the MFEM problem in a
manner consistent with the standard MOOSE Materials system.

!if-end!

!else
!include mfem/mfem_warning.md
