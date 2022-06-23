# Pressure Action System

!syntax description /BCs/Pressure/PressureAction

## Description

The Pressure Action, given in the input file as simply `Pressure`, is designed to simplify the input file when several variables have the same pressure boundary condition magnitude applied in the normal component.
Hydrostatic stress is a good example of a use case for the Pressure Action.

## Constructed MooseObjects

The Pressure Action is used to construct the [Pressure boundary condition](/Pressure.md) in all directions.

!table id=pressure_BC_action_table caption=Correspondence Among Action Functionality and MooseObjects
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| A pressure traction force | [Pressure BC](/Pressure.md) | `displacements` : a string of the displacement variables to which the Pressure BC should be applied |

The Pressure Action only applies the pressure traction in the same component direction as the `displacements` variables are listed.
That is, for the argument `displacements = 'disp_x disp_y disp_z'`, the Pressure Action will create three separate Pressure BCs:

- For the variable `disp_x`, the parameter setting `component = 0` is used
- For the variable `disp_y`, the parameter setting `component = 1` is used
- For the variable `disp_z`, the parameter setting `component = 2` is used

!alert note title=Displacement Variable-Component Relationship is Relative
Note that the location of each of the variables in the `displacements` string determines the value of the corresponding component.

As in the [Pressure](/Pressure.md) boundary condition, the  magnitude of the `Pressure` boundary condition can be specified as either a scalar (use the input parameter `factor`) or a `function` parameter.

The use of the automatic differentiation boundary condition [ADPressure](Pressure.md) can optionally
be selected via the `use_automatic_differentiation` input parameter.

## Example Input Syntax

!listing modules/tensor_mechanics/test/tests/pressure/pressure_test.i block=BCs/Pressure

!syntax parameters /BCs/Pressure/PressureAction

## Associated Actions

!syntax list /BCs/Pressure objects=True actions=False subsystems=False

!syntax list /BCs/Pressure objects=False actions=False subsystems=True

!syntax list /BCs/Pressure objects=False actions=True subsystems=False
