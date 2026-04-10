# MFEMExecutedObject

!if! function=hasCapability('mfem')

## Overview

`MFEMExecutedObject` is the base class for MFEM objects that are scheduled and executed by
[MFEMProblem](problem/MFEMProblem.md) during a solve. It derives from
[MFEMObject](base/MFEMObject.md), `SetupInterface`, and `DependencyResolverInterface`.

## Execution ordering

Execution order among `MFEMExecutedObject` instances is determined automatically by
`DependencyResolverInterface`: the MFEM scheduler builds a dependency graph from the variable,
postprocessor, and vector postprocessor names that each object declares it consumes or produces,
then performs a topological sort.

Derived class authors register dependency-bearing parameters at `validParams` time using the
provided helpers:

- `addDependencyParam<T>` — adds an optional parameter whose value names a consumed resource.
- `addRequiredDependencyParam<T>` — adds a required parameter whose value names a consumed resource.

The template parameter `T` must be one of `VariableName`, `std::vector<VariableName>`,
`PostprocessorName`, `std::vector<PostprocessorName>`, `VectorPostprocessorName`, or
`std::vector<VectorPostprocessorName>`. The scheduler infers the resource category from the type.

Produced resources are declared by overriding one or more of:

- `producedVariableNames()` — names of MFEM variables written by this object.
- `producedPostprocessorNames()` — names of postprocessors written by this object.
- `producedVectorPostprocessorNames()` — names of vector postprocessors written by this object.

## Lifecycle methods

Each `MFEMExecutedObject` has three lifecycle methods that `MFEMProblem` calls in order for every
execution pass:

| Method | Purpose |
| - | - |
| `initialize()` | Pre-execution setup (zero accumulators, clear state, etc.). |
| `execute()` | Main work: read variables/coefficients, write results. |
| `finalize()` | Post-execution cleanup or reduction (e.g., parallel reduction for postprocessors). |

All three have empty default implementations. Derived classes override whichever they need.

## Derived classes

The following MFEM object types derive from `MFEMExecutedObject`:

- [MFEMAuxKernel](auxkernels/MFEMAuxKernel.md) — updates real auxiliary variables.
- [MFEMComplexAuxKernel](auxkernels/MFEMComplexAuxKernel.md) — updates complex auxiliary variables.
- [MFEMPostprocessor](postprocessors/MFEMPostprocessor.md) — computes a scalar quantity.
- [MFEMVectorPostprocessor](vectorpostprocessors/MFEMVectorPostprocessor.md) — computes an array of values.
- [MFEMSubMeshTransfer](transfers/MFEMSubMeshTransfer.md) — transfers variable data between a mesh and a submesh.
- `MFEMInitialCondition` — sets the initial value on an MFEM variable.

!if-end!

!else
!include mfem/mfem_warning.md
