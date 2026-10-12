# DiscreteSymmetry

!if! function=hasCapability('mfem')

## Overview

`DiscreteSymmetry` is a virtual base class for objects that define transforms that represent
discrete symmetry operations on a target geometry.

Derived classes provide implementations for the `DiscreteSymmetry::ApplyTransform` method, that
transforms an input coordinate to a symmetrically equivalent coordinate.

Currently, two derived classes exist; `Moose::MFEM::TranslationalSymmetry` and
`Moose::MFEM::RotationalSymmetry`, used for imposing translational and rotational periodicity to the
mesh. These are used to reduce meshes containing these symmetries by identifying topologically
equivalent vertices. Importantly, all edges in the reduced mesh must remain unique; further
description is available [from the MFEM website](https://mfem.org/howto/periodic-boundaries/).

!if-end!

!else
!include mfem/mfem_warning.md
