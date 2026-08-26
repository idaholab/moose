# DiscreteSymmetry

!if! function=hasCapability('mfem')

## Overview

`DiscreteSymmetry` is a virtual base class for objects that define transforms that represent
discrete symmetry operations on a target geometry.

Derived classes provide implementations for the `DiscreteSymmetry::ApplyTransform` method, that
transforms an input coordinate to a symmetrically equivalent coordinate.

!if-end!

!else
!include mfem/mfem_warning.md
