# MFEMObject

!if! function=hasCapability('mfem')

## Overview

`MFEMObject` is the common base class for all MFEM objects. It provides every MFEM object with a
typed reference to the owning [MFEMProblem.md] and a set of helpers for
retrieving MFEM coefficients by name.

`MFEMObject` also mixes in `FunctionInterface`, `PostprocessorInterface`,
`VectorPostprocessorInterface`, and `ReporterInterface` so that derived classes can consume MOOSE
functions, postprocessors, vector postprocessors, and reporters in the normal way.

## Problem access

The `getMFEMProblem()` method returns a typed reference to the owning `MFEMProblem`.

## Coefficient access

MFEM coefficients declared through [MFEMFunctorMaterial.md]
objects are stored in the `CoefficientManager` owned by `MFEMProblem`. `MFEMObject` exposes two
families of accessor methods for them:

| Method | Looks up by |
| - | - |
| `getScalarCoefficient(param_name)` | Value of an `MFEMScalarCoefficientName` input parameter |
| `getVectorCoefficient(param_name)` | Value of an `MFEMVectorCoefficientName` input parameter |
| `getMatrixCoefficient(param_name)` | Value of an `MFEMMatrixCoefficientName` input parameter |
| `getScalarCoefficientByName(name)` | Declared coefficient name directly |
| `getVectorCoefficientByName(name)` | Declared coefficient name directly |
| `getMatrixCoefficientByName(name)` | Declared coefficient name directly |

The `param_name` variants are the most common: they read the coefficient name from an input
parameter, so the caller does not need to extract the string itself.

## Derived classes

`MFEMObject` is the root of the MFEM object hierarchy. Direct subclasses include:

- [MFEMExecutedObject.md] — objects that are scheduled and executed by `MFEMProblem` (aux kernels, postprocessors, transfers, initial conditions, etc.).
- [MFEMSolverBase.md] - MFEM base solver class. This is further sub-classed by:
  - [MFEMLinearSolverBase.md] — MFEM linear solver and preconditioner objects.
  - [MFEMNonlinearSolverBase.md] — MFEM nonlinear solver objects.
- `MFEMKernel` / `MFEMComplexKernel` — weak-form contributions to the equation system.
- `MFEMBoundaryCondition` — boundary conditions applied to the equation system.

!if-end!

!else
!include mfem/mfem_warning.md
