# PropertyMap

!if! function=hasCapability('mfem')

## Summary

`PropertyMap` stores MFEM coefficient objects, handling the set-up of piecewise coefficients defined
across multiple materials.

## Overview

`PropertyMap` is a templated class used to store `mfem::Coefficient`, `mfem::VectorCoefficient`, or
`mfem::MatrixCoefficient` derived objects added to the MFEM problem. It also associates added
coefficients for the same named property on multiple mesh subdomains (blocks) with the (global)
piecewise coefficients required by domain and boundary integrators that span multiple blocks.

Addition of new material properties and coefficients to the `PropertyMap` should usually be managed
by calling the appropriate methods of the `PropertyManager`.


!if-end!

!else
!include mfem/mfem_warning.md
