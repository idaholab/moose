# MFEM Executed Object Refactor

## Goal

Refactor MFEM executed subsystems so they no longer rely on the libMesh/MOOSE user object
execution model. The target is a clean MFEM-native execution graph with automatic dependency
resolution.

## Scope

The executed MFEM families to unify are:

- `MFEMInitialCondition`
- `MFEMAuxKernel`
- `MFEMComplexAuxKernel`
- `MFEMSubMeshTransfer`
- `MFEMPostprocessor`
- `MFEMVectorPostprocessor`

Non-executed MFEM families should remain on direct `MooseObject`/warehouse-backed storage.

## Target Architecture

- `MFEMExecutedObject` is the common base for all executed MFEM families.
- `MFEMGeneralUserObject` is removed.
- MFEM executed objects do not participate in the traditional MOOSE user object scheduler.
- `execution_order_group` is removed.
- Ordering is derived automatically from dependencies.

## Dependency Model

MFEM executed scheduling should be based on a dependency DAG built per `execute_on` flag.

Each `MFEMExecutedObject` should declare or expose:

- what it produces
- what it consumes

The important resource classes are:

- MFEM variable values
- MFEM submesh-transferred variable values
- postprocessor values
- vector postprocessor values
- reporter values
- functions/coefficient-backed dependencies where needed

Typical examples:

- an IC produces a variable value
- an aux kernel consumes one or more source variables and produces its target variable
- a submesh transfer consumes a source variable and produces a destination variable
- a postprocessor consumes variables and produces a postprocessor value
- a vector postprocessor consumes variables and produces vector postprocessor values

The scheduler should:

1. collect MFEM executed objects for a given execute flag
2. infer dependency edges
3. topologically sort the graph
4. error clearly on cycles or unresolved dependencies

## Design Intent

This refactor is explicitly intended to avoid the historical libMesh/MOOSE dependency problems
between ICs, aux kernels, user objects, postprocessors, and vector postprocessors.

The MFEM side should be a fresh start:

- no accidental ordering through inheritance
- no manual execution groups
- no overloading of the user object system for semantically distinct subsystems

## Implementation Stages

1. Finish converging all executed MFEM families on `MFEMExecutedObject`.
2. Remove remaining `MFEMGeneralUserObject` usage.
3. Introduce a unified MFEM executed-object scheduler in `MFEMProblem`.
4. Add automatic dependency extraction and topological sorting.
5. Remove `execution_order_group` from MFEM executed classes and input files.
6. Update MFEM tests to rely on automatic ordering only.

## Current Caution

Until the unified dependency scheduler exists, splitting executed MFEM families across different
execution paths can create ordering regressions. Intermediate implementations should be judged by
how well they preserve or improve dependency behavior, not just by cleaner inheritance.
