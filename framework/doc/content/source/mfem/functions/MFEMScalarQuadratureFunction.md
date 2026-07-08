# MFEMScalarQuadratureFunction

!if! function=hasCapability('mfem')

## Overview

`MFEMScalarQuadratureFunction` declares a named scalar MFEM coefficient that holds precomputed
values of a source scalar coefficient at the quadrature points of an
[MFEM QuadratureSpace](https://docs.mfem.org/html/classmfem_1_1QuadratureSpace.html) of the
requested integration rule order. The source may be any named scalar coefficient in the problem,
including parsed functions and (grid)functions of problem variables. The declared coefficient can
be used wherever a scalar coefficient name is accepted, in place of the source coefficient.

Values are projected from the source coefficient lazily, on the first evaluation of the declared
coefficient after its stored values have been invalidated. The `updates` parameter controls when
invalidation occurs:

- `none`: the source never changes after initialization; it is projected exactly once.
- `time`: the source changes with time only; it is re-projected when the simulation time is set,
  at most once per solve.
- `solve` (default): the source may additionally depend on solution variables; it is also
  re-projected whenever trial variables are updated, including between nonlinear iterations.

!alert warning
The integration rule used by objects consuming this coefficient must match the quadrature rule
implied by the `order` parameter exactly. The underlying
[QuadratureFunctionCoefficient](https://docs.mfem.org/html/classmfem_1_1QuadratureFunctionCoefficient.html)
looks values up by quadrature point index, so evaluating it with a different integration rule
silently returns values belonging to different points. For example, `mfem::DomainLFIntegrator`
uses a rule of order $2p$ by default for elements of order $p$, whereas
`mfem::DiffusionIntegrator` uses order $2p + d - 1$ on tensor-product elements of dimension $d$.

## Example Input File Syntax

!listing test/tests/mfem/functions/quadrature_function_source.i block=QuadratureFunctions Kernels

!syntax parameters /QuadratureFunctions/MFEMScalarQuadratureFunction

!syntax inputs /QuadratureFunctions/MFEMScalarQuadratureFunction

!syntax children /QuadratureFunctions/MFEMScalarQuadratureFunction

!if-end!

!else
!include mfem/mfem_warning.md
