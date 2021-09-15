# SetupTimeIntegratorAction

!syntax description /Executioner/TimeIntegrator/SetupTimeIntegratorAction

A `TimeIntegrator` is specified as an object inside the `[TimeIntegrator]` block with the `[Executioner]` block
as shown below

!listing test/tests/variables/time_derivatives_neighbor/test.i block=Executioner

This [MooseObjectAction.md] adds them to the [Problem](syntax/Problem/index.md).
More information about TimeIntegrators may be found on the
[TimeIntegrator syntax documentation](syntax/Executioner/TimeIntegrator/index.md).

!syntax parameters /Executioner/TimeIntegrator/SetupTimeIntegratorAction
