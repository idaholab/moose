# Pressure Action

## Description

The `Pressure` Action is used to create a set of pressure boundary conditions for a string of displacement variables; the typical use case for this action is the application of hydrostatic pressure. See the description, example use, and parameters on the [Pressure](/Pressure/index.md) action system page.

!alert warning
When using the [Controls system](syntax/Controls/index.md) to control the active status of Pressure boundary conditions,
the [Pressure.md] boundary conditions created by the `Pressure Action` cannot be controlled individually,
they will all follow the same enabled/disabled status.