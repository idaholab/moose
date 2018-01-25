# Pressure Action
!syntax description /BCs/Pressure/PressureAction

## Description
The Pressure Action, given in the input file as simply `Pressure`, is designed to simplify the input file when several variables have the same pressure boundary condition magnitude applied in the normal component.
Hydrostatic stress is a good example of a use case for the Pressure Action.

## Constructed MooseObjects
The Pressure Action is used to construct only the [Pressure boundary condition](/BCs/tensor_mechanics/Pressure.md).

!table id=pressure_BC_action_table caption=Correspondence Among Action Functionality and MooseObjects
| Functionality     | Replaced Classes   | Associated Parameters   |
|-------------------|--------------------|-------------------------|
| A pressure traction force | [Pressure BC](/BCs/tensor_mechanics/Pressure.md) | `displacements` : a string of the displacement variables to which the Pressure BC should be applied |

The Pressure Action only applies the pressure traction in the same component direction as the `displacements` variables are listed.
That is, for the argument `displacements = 'disp_x disp_y disp_z'`, the Pressure Action will create three separate Pressure BCs:

  - For the variable `disp_x`, the parameter setting `component = 0` is used
  - For the variable `disp_y`, the parameter setting `component = 1` is used
  - For the variable `disp_z`, the parameter setting `component = 2` is used

Note that the placement of the variables in the `displacements` string determines the value of the component.

As in the [Pressure](/BCs/tensor_mechanics/Pressure.md) boundary condition, the  magnitude of the `Pressure` boundary condition can be specified as either a scalar (use the input parameter `factor`) or a `function` parameter.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/pressure/pressure_test.i block=BCs/Pressure

!syntax parameters /BCs/Pressure/PressureAction
