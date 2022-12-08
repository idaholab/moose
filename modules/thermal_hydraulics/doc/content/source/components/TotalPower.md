# TotalPower

This component is a [power component](thermal_hydraulics/component_groups/power.md)
that specifies the power as a constant value via a user parameter.

## Usage

The user provides a power value via the parameter
[!param](/Components/TotalPower/power). This parameter may be controlled via the
[ControlLogic system](ControlLogic/index.md).

!syntax parameters /Components/TotalPower

## Variables

This component creates the following auxiliary scalar variable, where `<cname>`
is the name of the component:

| Variable | Description |
| :- | :- | :- |
| `<cname>:power` | Power \[W\] |

!syntax inputs /Components/TotalPower

!syntax children /Components/TotalPower
