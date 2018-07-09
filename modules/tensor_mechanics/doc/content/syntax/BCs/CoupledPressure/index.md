# Coupled Pressure Action System

!syntax description /BCs/CoupledPressure/CoupledPressureAction

## Description

The Coupled Pressure Action, given in the input file as simply `CoupledPressureBC`, is designed to simplify the input file when several variables have the same pressure boundary condition magnitude applied in the normal component.
Transfer of pressure variable from an multi-app is a good example of a use case for the Coupled Pressure Action.

## Constructed MooseObjects

The Coupled Pressure Action is used to construct only the [Coupled Pressure boundary condition](/CoupledPressureBC.md).

!table id=pressure_BC_action_table caption=Correspondence Among Action Functionality and MooseObjects
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| A pressure traction force given by a variable | [Coupled Pressure BC](/CoupledPressureBC.md) | `displacements` : a string of the displacement variables to which the Coupled Pressure BC should be applied |
|  |  | `pressure` : a variable prescribing the pressure to be applied |

The Pressure Action only applies the pressure traction in the same component direction as the `displacements` variables are listed.
That is, for the argument `displacements = 'disp_x disp_y disp_z'`, the Pressure Action will create three separate Pressure BCs:

- For the variable `disp_x`, the parameter setting `component = 0` is used
- For the variable `disp_y`, the parameter setting `component = 1` is used
- For the variable `disp_z`, the parameter setting `component = 2` is used

!alert note title=Displacement Variable-Component Relationship is Relative
Note that the location of each of the variables in the `displacements` string determines the value of the corresponding component.

## Example Input Syntax

!listing modules/tensor_mechanics/test/tests/coupled_pressure/coupled_pressure_test.i block=BCs/CoupledPressure

!syntax parameters /BCs/CoupledPressure/CoupledPressureAction

## Associated Actions

!syntax list /BCs/CoupledPressure objects=True actions=False subsystems=False

!syntax list /BCs/CoupledPressure objects=False actions=False subsystems=True

!syntax list /BCs/CoupledPressure objects=False actions=True subsystems=False
