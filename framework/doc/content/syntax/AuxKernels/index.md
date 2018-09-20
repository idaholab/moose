<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# AuxKernels System

## Overview

- `AuxKernel` objects should go under `include/auxkernels` and `src/auxkernels`.
- They are similar to regular kernels except that they override `computeValue()` instead of `computeQpResidual()`.
- They don't have Jacobians.
- Note, there is no difference between a nodal auxiliary kernel and an elemental.
- The difference is only the input file.
- An `AuxKernel` operates on an Auxiliary Variable.

## (Some) Values Available to AuxKernels

- `_u, _grad_u`

  - Value and gradient of variable this AuxKernel is operating on.

- `_q_point`

  - XYZ coordinates of the current q-point.
  - Only valid for element AuxKernel !

- `_qp`

  - Current quadrature point.
  - Used even for nodal AuxKernels! (Just for consistency)

- `_current_elem`

  - A pointer to the current element that is being operated on.
  - Only valid for element AuxKernels!

- `_current_node`

  - A pointer to the current node that is being operated on.
  - Only valid for element AuxKernels!

- And more !

## Example 10

Look at [Example 10](ex10_aux.md)

## Further AuxKernel documentation

!syntax list /AuxKernels objects=True actions=False subsystems=False

!syntax list /AuxKernels objects=False actions=False subsystems=True

!syntax list /AuxKernels objects=False actions=True subsystems=False

