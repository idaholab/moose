# MFEMPostprocessor

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM postprocessors used to evaluate a single scalar.

## Overview

MFEM postprocessors calculate scalar quantities from the (aux)variables, typically after each
timestep.

An `MFEMPostprocessor` is derived from `MFEMExecutedObject`. Its ordering relative to MFEM initial
conditions, aux kernels, transfers, and other MFEM postprocessors is determined automatically from
detected data dependencies instead of manual execution groups.

The value of a postprocessor added to an MFEM problem is available as a spatially uniform scalar
coefficient named after it, and so may be used anywhere a scalar coefficient is expected. This
applies to postprocessors of all types, not only to those derived from `MFEMPostprocessor`, so that
values calculated elsewhere and transferred in, such as from a subapp into a
[Receiver.md] postprocessor, may also be used to build coefficients.

Such a coefficient reads the postprocessor's value whenever it is evaluated, and does not itself
register a data dependency on the postprocessor supplying it. An object consuming the coefficient
on the same execution flag as the postprocessor is executed on is therefore not ordered after it,
and will see the value from the previous execution. Where the two would otherwise coincide, execute
the supplying postprocessor on an earlier flag, as in the example below:

!listing test/tests/mfem/functions/postprocessor_coefficient.i block=Postprocessors Kernels

`MFEMPostprocessor` is a purely virtual base class. Derived classes
should override the `execute` and `getValue` methods.

!if-end!

!else
!include mfem/mfem_warning.md
