# Peridynamic Master Action System

## Description

The peridynamics mechanics Action is a convenience object that simplifies part of the
mechanics system setup. It sets up force density integral Kernels for all displacements at once.

## Constructed MooseObjects

!table id=pd_mechanics_action_table caption=Correspondence Among Action Functionality and MooseObjects for the `MechanicsActionPD` Action
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Mechanics equilibrium conditions | [Bond-based Small Strain Models](/kernels/MechanicsBPD.md) or [Ordinary State-based Small Strain Models](/kernels/MechanicsOSPD.md) or [Force-stabilized Non-ordinary State-based Small Strain Model](/kernels/ForceStabilizedSmallStrainMechanicsNOSPD.md) or Horizon-stabilized Non-ordinary State-based Small Strain Models: [Form I]](/kernels/HorizonStabilizedFormISmallStrainMechanicsNOSPD.md); [Form II]](/kernels/HorizonStabilizedFormIISmallStrainMechanicsNOSPD.md) or Horizon-stabilized Non-ordinary State-based Finite Strain Models: [Form I](/kernels/HorizonStabilizedFormIFiniteStrainMechanicsNOSPD.md); [Form II](/kernels/HorizonStabilizedFormIIFiniteStrainMechanicsNOSPD.md) | `displacements` : a string of the displacement field variables; `temperature`: a string of the temperature field variable |
| Ghost bonds for nonlocal computation | [Ghost Element UserObject](/GhostElemPD.md) | None |
| Setup quadrature rule | [Variables](syntax/Variables/index.md) | `type`: GAUSS_LOBATTO; `order`: FIRST |
| Add AuxVariable for bond status | [AuxVariables](/AuxVariables/index.md) | `initial_condition` is set to 1 |


## Example Input Syntax

### Subblocks

The subblocks of the Mechanics action are what triggers MOOSE objects to be built. If none of the mechanics is subdomain restricted a single subblock will be used.

!listing modules/peridynamics/test/tests/simple_tests/2D_finite_strain_H1NOSPD.i block=Modules/Peridynamics/Mechanics/Master

If different mechanics models are needed, multiple subblocks with subdomain restrictions can be used.

```
[Modules/Peridynamics/Mechanics/Master]
  [./block_a]
    ...
  [../]
  [./block_b]
    ...
  [../]
[]
```

Parameters supplied at the `[Modules/Peridynamics/Mechanics/Master]` level act as defaults for the Mechanics action subblocks.

!syntax parameters /Modules/Peridynamics/Mechanics/Master/MechanicsActionPD


## Associated Actions

!syntax list /Modules/Peridynamics/Mechanics/Master objects=True actions=False subsystems=False

!syntax list /Modules/Peridynamics/Mechanics/Master objects=False actions=False subsystems=True

!syntax list /Modules/Peridynamics/Mechanics/Master objects=False actions=True subsystems=False
