# Shell Element Action System

!syntax description /Modules/TensorMechanics/ShellElementMaster/ShellElementAction

## Description

The ShellElement Action is a convenience object that simplifies part of the
mechanics system setup for shell elements.

## Shell Elements: Constructed MooseObjects

The `ShellElement` Action can be used to construct the kernels, strain materials, and displacement variables for a simulation using +Shell Elements+.

!table id=shell_Element_action_table caption=Correspondence Among Action Functionality and MooseObjects for the `ShellElement` Action used in a +Shell Elements+ Simulation
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| Add the displacement variables | [Variables](syntax/Variables/index.md) | `add_variables`: boolean |
| Calculation of stress divergence for a +Shell+ element | [ADStressDivergenceShell](/ADStressDivergenceShell.md) | `displacements` : a string of the displacement variables |

## Example Input Syntax

!listing modules/tensor_mechanics/test/tests/shell/static/beam_bending_3d.i block=Modules/TensorMechanics/ShellElementMaster
``
### Subblocks

The subblocks of the ShellElement action are what trigger MOOSE objects to be built.
If none of the mechanics is subdomain restricted a single subblock should be used, yet
if different mechanics models are needed, multiple subblocks with subdomain restrictions
can be used.

Parameters supplied at the `[Modules/TensorMechanics/ShellElementMaster]` level act as
defaults for all the subblocks within that ShellElement block.

!syntax parameters /Modules/TensorMechanics/ShellElementMaster/ShellElementAction

## Associated Actions

!syntax list /Modules/TensorMechanics/ShellElementMaster objects=True actions=False subsystems=False

!syntax list /Modules/TensorMechanics/ShellElementMaster objects=False actions=False subsystems=True

!syntax list /Modules/TensorMechanics/ShellElementMaster objects=False actions=True subsystems=False
