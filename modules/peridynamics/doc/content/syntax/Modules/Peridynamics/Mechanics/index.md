# Peridynamic Mechanics Action System

## Description

The peridynamics mechanics Action is a convenience object that simplifies part of the
mechanics system setup. It sets up force density integral Kernels for all displacements at once.

## Constructed MooseObjects

!table id=pd_mechanics_action_table caption=Correspondence Among Action Functionality and MooseObjects for the `MechanicsActionPD` Action
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Mechanics equilibrium conditions | [Bond-based Mechanics Models](/kernels/MechanicsBPD.md) or [Ordinary State-based Mechanics Models](/kernels/MechanicsOSPD.md) or [Force-stabilized Correspondence Material Model](/kernels/ForceStabilizedSmallStrainMechanicsNOSPD.md) or [Bond-associated Correspondence Small Strain Model](/kernels/SmallStrainMechanicsNOSPD.md) or [Bond-associated Correspondence Finite Strain Model](/kernels/FiniteStrainMechanicsNOSPD.md) | `displacements` : a string of the displacement field variables; `temperature`: a string of the temperature field variable |
| Ghost bonds for nonlocal computation |[Ghost Element UserObject](/GhostElemPD.md)| None |
| Setup quadrature rule | [Variables](syntax/Variables/index.md) | `type`: GAUSS_LOBATTO; `order`: FIRST |
| Add AuxVariable for bond status | [AuxVariables](/AuxVariables/index.md) | `initial_condition` is set to 1 |


## Example Input Syntax

### Subblocks

The subblocks of the Mechanics action are what triggers MOOSE objects to be built. If none of the mechanics is subdomain restricted a single subblock will be used.

!listing modules/peridynamics/test/tests/solid_mechanics_basics/2d_non_ordinary_state_finite_strain.i block=Modules/Peridynamics/Mechanics

If different mechanics models are needed, multiple subblocks with subdomain restrictions can be used.

```
[Modules/Peridynamics/Mechanics]
  [./block_a]
    ...
  [../]
  [./block_b]
    ...
  [../]
[]
```

Parameters supplied at the `[Modules/Peridynamics/Mechanics]` level act as defaults for the Mechanics action subblocks.

!syntax parameters /Modules/Peridynamics/Mechanics/MechanicsActionPD


## Associated Actions

!syntax list /Modules/Peridynamics/Mechanics objects=True actions=False subsystems=False

!syntax list /Modules/Peridynamics/Mechanics objects=False actions=False subsystems=True

!syntax list /Modules/Peridynamics/Mechanics objects=False actions=True subsystems=False
