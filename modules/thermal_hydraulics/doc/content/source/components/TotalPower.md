# TotalPower

This component is a [power component](thermal_hydraulics/component_groups/power.md)
that specifies the power via a user-supplied function.

## Usage

The user provides the power via the function parameter
[!param](/Components/TotalPower/power). A constant value may be supplied directly;
to vary the power during a simulation, provide a function of time.

!syntax parameters /Components/TotalPower

## Variables

This component creates the following auxiliary scalar variable, where `<cname>`
is the name of the component:

| Variable | Description |
| :- | :- | :- |
| `<cname>:power` | Power \[W\] |

!syntax inputs /Components/TotalPower

!syntax children /Components/TotalPower
