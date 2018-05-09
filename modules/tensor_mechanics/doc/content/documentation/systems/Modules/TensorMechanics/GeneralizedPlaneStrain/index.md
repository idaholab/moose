# GeneralizedPlaneStrain Action

Sets up a generalized plane strain model. This action can be generated using the [TensorMechanics Master action](../Master). A detailed description of generalized plane strain model can be found in the [formulation](tensor_mechanics/generalized_plane_strain.md) page.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/generalized_plane_strain/generalized_plane_strain_squares.i block=Modules/TensorMechanics/GeneralizedPlaneStrain

## Subblocks

The subblocks of the GeneralizedPlaneStrain action are what triggers MOOSE objects to be built.
If a generalized plane strain model is applied for the whole simulation domain, a single subblock should be used

```
[Modules/TensorMechanics/GeneralizedPlaneStrain]
  [./all]
    ...
  [../]
[]
```

if different mesh subdomain has different generalized plane strain model, multiple subblocks with subdomain restrictions can be used.

```
[Modules/TensorMechanics/GeneralizedPlaneStrain]
  [./block_a]
    ...
  [../]
  [./block_b]
    ...
  [../]
[]
```

## Toplevel parameters

Parameters supplied at the `[Modules/TensorMechanics/GeneralizedPlaneStrain]` level act as
defaults for the Master action subblocks.

!syntax list /Modules/TensorMechanics/GeneralizedPlaneStrain objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/GeneralizedPlaneStrain objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/GeneralizedPlaneStrain objects=False actions=True subsystems=False
