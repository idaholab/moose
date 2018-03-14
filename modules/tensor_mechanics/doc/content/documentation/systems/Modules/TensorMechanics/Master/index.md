# Master Action

The TensorMechanics Master Action is a convenience object that simplifies part of the
mechanics system setup. It performs

- Add StressDivergence Kernels (for the current coordinate system)
- Add Strain calculation material (for the chosen strain model)
- Correctly set use of displaced mesh
- Optional: Setup of displacement variables (with the correct order for the current mesh)
- Optional: Add AuxVariables and AuxKernels for various tensor comonent and quantity outputs
- Optional: Set up out-of-plane stress/strain consistently

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/action/two_block_new.i block=Modules/TensorMechanics/Master

## Subblocks

The subblocks of the Master action are what triggers MOOSE objects to be built.
If none of the mechanics is subdomain restricted a single subblock will be used

```
[Modules/TensorMechanics/Master]
  [./all]
    ...
  [../]
[]
```

if different mechanics models are needed, multiple subblocks with subdomain restrictions
can be used.

```
[Modules/TensorMechanics/Master]
  [./block_a]
    ...
  [../]
  [./block_b]
    ...
  [../]
[]
```

## Toplevel parameters

Parameters supplied at the `[Modules/TensorMechanics/Master]` level act as
defaults for the Master action subblocks.

!syntax list /Modules/TensorMechanics/Master objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/Master objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/Master objects=False actions=True subsystems=False
