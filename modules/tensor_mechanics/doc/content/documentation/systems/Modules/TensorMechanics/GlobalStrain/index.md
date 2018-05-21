# Global Strain Action

Sets up a model for global strain and corresponding displacement calculation. This action can be created using the the following syntax.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain_action.i block=Modules/TensorMechanics/GlobalStrain

## Subblocks

The subblocks of the GlobalStrain action triggers MOOSE objects to be built.
It can be applied to the whole domain using a single subblock

```
[Modules/TensorMechanics/GlobalStrain]
  [./all]
    ...
  [../]
[]
```

or multiple subblocks can be used to apply block restrictions to the objects

```
[Modules/TensorMechanics/GlobalStrain]
  [./block_a]
    ...
  [../]
  [./block_b]
    ...
  [../]
[]
```

!syntax list /Modules/TensorMechanics/GlobalStrain objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/GlobalStrain objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/GlobalStrain objects=False actions=True subsystems=False
