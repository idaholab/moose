# MFEMCoordinateCoefficients

!if! function=hasCapability('mfem')

## Summary

An abstract base class for coordinate dependent MFEM coefficients.

## Overview

`MFEMCoordinateCoefficients` provides an interface for defining coordinate system dependent scalar coefficients that can be exposed to MFEM through
the `CoefficientManager` .

Objects derived from this class are intended to be created under the top level `[Coordinates]` block which are responsible for constructing
and exposing built in scalar coefficients associated with a coordinate system, derived from the MFEM [`mfem::Coefficient`](https://docs.mfem.org/html/classmfem_1_1Coefficient.html) interface.

`MFEMCoordinateCoefficients` is a bse class. Derived classes should override `build()` to construct any required coefficients and `getBuiltinCoefficient(const std::string & name) const` to provide name based lookup of those coefficients. This allows coordinate depedent coefficients to be used through the normal MFEM scalar coefficient path, ie. `MFEMGenericFunctorMaterial` property definitions or MFEM kernels.

!if-end!

!else
!include mfem/mfem_warning.md
