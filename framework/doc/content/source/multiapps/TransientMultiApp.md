# TransientMultiApp

!syntax description /MultiApps/TransientMultiApp

## Overview

The TransientMultiApp is designed to perform simulations with sub-applications that progress in
time with the main application.  A `TransientMultiApp` requires that your "sub-apps" use an
`Executioner` derived from `Transient`.

A `TransientMultiApp` MultiApp objects will be taken into account during time step selection inside
the "main" `Transient` executioner.  By default, the minimum time step over the main and all
sub-apps is utilized. The ability to perform sub-cycling, which allows the sub-applications
to perform multiple time steps per execution may be enabled using the
[!param](/MultiApps/TransientMultiApp/sub_cycling) parameter.
When performing sub-cycling, transferred auxiliary variables on sub-apps are allowed to be
interpolated between the start time and the end time of the main app with
[!param](/MultiApps/TransientMultiApp/interpolate_transfers) parameter.

## Time state of TransientMultiApps

`TransientMultiApps` are "auto-advanced" by default whenever we are not doing
Picard iterations between the main and sub-application. This means that the
`Transient::endStep` and `Transient::postStep` methods of the sub-applications
executioner are called, regardless of whether the sub-application solve fails or
not. The `endStep` method increments the time and also performs
`EXEC_TIMESTEP_END` output. When sub-applications are auto-advanced, their
`endStep` call happens before the main application's `endStep` call. This has
the important benefit that when main application output occurs, the
sub-application's and main application's time states are the same, which
enables MOOSE restart/recovery capability.

## Handing sub-application solve failures

As noted above, the default behavior when running `TransientMultiApps` is that
their time state is incremented, e.g. they are "auto-advanced", regardless of
whether their solve is actually successful. This is undesirable behavior, but we
believe that the syncing of main and sub-application states, under normal
operation, to enable correct [checkpoint](/Checkpoint.md) output is a good
trade. Given the constraints of the elected design, there are still multiple ways to turn a failed
sub-application solve from a warning into an exception that will force corrective
behavior in either the sub- or main-application:

1. The user can set `auto_advance = false` in the `Executioner` block of the
   main application . This will cause the main application to immediately cut
   its time-step when the sub-application fails. **However**, setting this
   parameter to `false` also eliminates the possibility of doing restart/recover
   because the main and sub will be out of sync if/when checkpoint output occurs.
2. The user can set `catch_up = true` in the `TransientMultiApp` block. This
   will cause the sub-application to try and catch up to the main application
   after a sub-app failed solve. If catch-up is unsuccessful, then MOOSE
   registers this as a true failure of the solve, and the main dt will *then*
   get cut. This option has the advantage of keeping the main and sub
   transient states in sync, enabling accurate restart/recover data.

In general, if the user wants sub-application failed solves to be treated as
exceptions, we recommend that option 2 over option 1.

## Example Input File Syntax

The following input file shows the creation of a TransientMultiApp object with the time step
size being governed by the main application.

!listing transient_multiapp/dt_from_parent.i block=MultiApps

!syntax parameters /MultiApps/TransientMultiApp

!syntax inputs /MultiApps/TransientMultiApp

!syntax children /MultiApps/TransientMultiApp
