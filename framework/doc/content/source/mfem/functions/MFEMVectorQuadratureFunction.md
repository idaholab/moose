# MFEMVectorQuadratureFunction

!if! function=hasCapability('mfem')

## Overview

`MFEMVectorQuadratureFunction` declares a named vector MFEM coefficient that holds precomputed
values of a source vector coefficient at the quadrature points of an
[MFEM QuadratureSpace](https://docs.mfem.org/html/classmfem_1_1QuadratureSpace.html) of the
requested integration rule order. It is the vector-valued counterpart of
[MFEMScalarQuadratureFunction.md]; the vector dimension of the stored values is taken from the
source coefficient. The declared coefficient can be used wherever a vector coefficient name is
accepted, in place of the source coefficient.

As for the scalar variant, values are projected from the source coefficient lazily, and the
`updates` parameter controls when the stored values are invalidated and re-projected:

- `none`: the source never changes after initialization; it is projected exactly once.
- `time`: the source changes with time only; it is re-projected when the simulation time is set,
  at most once per solve.
- `solve` (default): the source may additionally depend on solution variables; it is also
  re-projected whenever trial variables are updated, including between nonlinear iterations.

!alert warning
The integration rule used by objects consuming this coefficient must match the quadrature rule
implied by the `order` parameter exactly. The underlying
[VectorQuadratureFunctionCoefficient](https://docs.mfem.org/html/classmfem_1_1VectorQuadratureFunctionCoefficient.html)
looks values up by quadrature point index, so evaluating it with a different integration rule
silently returns values belonging to different points. For example,
`mfem::VectorDomainLFIntegrator` uses a rule of order $2p$ by default for elements of order $p$.

## Example Input File Syntax

!listing test/tests/mfem/kernels/gravity.i block=Kernels

!syntax parameters /QuadratureFunctions/MFEMVectorQuadratureFunction

!syntax inputs /QuadratureFunctions/MFEMVectorQuadratureFunction

!syntax children /QuadratureFunctions/MFEMVectorQuadratureFunction

!if-end!

!else
!include mfem/mfem_warning.md
