# TransientMultiApp

!syntax description /MultiApps/TransientMultiApp

## Overview

The TransientMultiApp is designed to perform simulations with sub-applications that progress in
time with the master application.  A `TransientMultiApp` requires that your "sub-apps" use an
`Executioner` derived from `Transient`.

A `TransientMultiApp` MultiApp objects will be taken into account during time step selection inside
the "master" `Transient` executioner.  By default, the minimum time step over the master and all
sub-apps is utilized. The ability to do perform sub-cycling, which allows the sub-applications
to perform multiple time steps per execution may be enabled using the
[!param](/MultiApps/TransientMultiApp/sub_cycling) parameter.

## Example Input File Syntax

The following input file shows the creation of a TransientMultiApp object with the time step
size being governed by the master application.

!listing transient_multiapp/dt_from_master.i block=MultiApps

!syntax parameters /MultiApps/TransientMultiApp

!syntax inputs /MultiApps/TransientMultiApp

!syntax children /MultiApps/TransientMultiApp
