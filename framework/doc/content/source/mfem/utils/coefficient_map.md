# CoefficientMap

!if! function=hasCapability('mfem')

## Summary

`CoefficientMap` stores MFEM coefficient objects and handles the
set-up of properties defined
across multiple materials, represented by piecewise coefficients.

## Overview

`CoefficientMap` is a templated class used to store `mfem::Coefficient`, `mfem::VectorCoefficient`, or
`mfem::MatrixCoefficient` derived objects added to the MFEM problem. It also associates added
coefficients for the same named property on multiple mesh subdomains (blocks) with the (global)
piecewise coefficients required by domain and boundary integrators that span multiple blocks.

Addition of new material properties and coefficients to the `CoefficientMap` should usually be managed
by calling the appropriate methods of the `CoefficientManager`.

!if-end!

!else
!include mfem/mfem_warning.md
