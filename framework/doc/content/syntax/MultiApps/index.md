# MultiApp System

## Overview

MOOSE was originally created to solve fully-coupled systems of [!ac](PDEs), but not all systems need to
be or are fully coupled:

- multiscale systems are generally loosely coupled between scales;
- systems with both fast and slow physics can be decoupled in time; and
- simulations involving input from external codes might be solved somewhat decoupled.

To MOOSE these situations look like loosely-coupled systems of fully-coupled equations. A `MultiApp`
allows you to simultaneously solve for individual physics systems.

Each sub-application (app) is considered independent. There is always a
"main" app that is doing the primary solve. The "main" app can then have any number of
`MultiApp` objects. Each `MultiApp` can represent many (hence Multi) "sub-applications" (sub-apps).
The sub-apps can be solving for completely different physics from the main application.  They can be
other MOOSE applications, or might represent external applications. A sub-app can, itself, have
`MultiApps`, leading to multi-level solves, as shown below.

!media multi_apps/multiapp_hierarchy.png id=multiapps style=width:60%;margin-left:auto;margin-right:auto;
       caption=Example multi-level MultiApp hierarchy.

## Input File Syntax

`MultiApp` objects are declared in the `[MultiApps]` block and require a "type" just like any other block.

The [!param](/MultiApps/TransientMultiApp/app_type) is the name of the `MooseApp` derived app that is going
to be executed. Generally, this is the name of the application being
executed, therefore if this parameter is omitted it will default as such. However this system
is designed for running another applications that are compiled or linked into the current app.

Sub-apps are created when a MultiApp is added by MOOSE.

A `MultiApp` can be executed at any point during the main solve by setting the
[!param](/MultiApps/TransientMultiApp/execute_on) parameter.
MultiApps at the same point are executed sequentially.
Before the execution, data on the main app are transferred to sub-apps of all the MultiApps and data on sub-apps are transferred back after the execution.
The execution order of all MultiApps at the same point is not determined.
The order is also irrelevant because no data transfers directly among MultiApps.
To enforce the ordering of execution, users can use multi-level MultiApps or set the MultiApps executed at different points.
If a `MultiApp` is set to be executed on timestep_begin or timestep_end, the formed loosely-coupled systems of fully-coupled
equations can be solved with [Fixed Point iterations](syntax/Executioner/index.md).

!listing multiapps/transient_multiapp/dt_from_parent.i block=MultiApps

## Positions id=multiapp-positions

The [!param](/MultiApps/TransientMultiApp/positions) parameter is a coordinate offset from
the main app domain to the sub-app domain, as illustrated by [multiapps_pos]. The parameter
requires the positions to be provided as a set of $(x, y, z)$ coordinates for each sub-app.

The number of coordinate sets determines the actual number of sub-applications created.  If there is
a large number of positions a file can be provided instead using the
[!param](/MultiApps/TransientMultiApp/positions_file) parameter.


- The $(x, y, z)$ coordinates are a vector that is being added to the coordinates of the sub-app's
  domain to put that domain in a specific location within the main domain.
- If the sub-app's domain starts at $(0,0,0)$ it is easy to think of moving that point around
  using [!param](/MultiApps/TransientMultiApp/positions).
- For sub-apps on completely different scales, `positions` is the point in the main domain where
  that app is located.

!media framework/multiapps_positions.png id=multiapps_pos style=width:80%;margin-left:auto;margin-right:auto;
       caption=Example of MultiApp object position.

If this parameter is not provided, a single position (0,0,0) will be used.

## Mesh optimizations

The [!param](/MultiApps/TransientMultiApp/clone_parent_mesh) parameter allows for re-using the
main application mesh in the sub-app. This avoids repeating mesh creation operations. This does
not automatically transfer the mesh modifications performed by [Adaptivity](syntax/Adaptivity/index.md)
on either the main or sub-app, though it does transfer initial mesh modification work such as uniform
refinement.

When using the same mesh between two applications, the [MultiAppCopyTransfer.md] may be
utilized for more efficient transfers of field variables.

## Parallel Execution

The `MultiApp` system is designed for efficient parallel execution of hierarchical problems. The
main application utilizes all processors.  Within each `MultiApp`, all of the processors are split
among the sub-apps. If there are more sub-apps than processors, each processor will solve for
multiple sub-apps.  All sub-apps of a given `MultiApp` are run simultaneously in parallel. Multiple
`MultiApps` will be executed one after another.


## Dynamically Loading MultiApps

If building with dynamic libraries (the default) other applications can be loaded without adding them
to your Makefile and registering them. Simply set the proper `type` in your input file
(e.g. `AnimalApp`) and MOOSE will attempt to find the other library dynamically.

- The path (relative preferred) can be set in your input file using the parameter
  [!param](/MultiApps/TransientMultiApp/library_path); this path needs to point to the `lib` folder
  within an application directory.
- The `MOOSE_LIBRARY_PATH` may also be set to include paths for MOOSE to search.


!alert warning
Each application must be compiled separately since the main application Makefile does not have
knowledge of any sub-app application dependencies.

## Restart and Recover

General information about restart/recover can be found at [Restart/Recovery](restart_recover.md optional=True).
When running a multiapp simulation you do not need to enable checkpoint output in each sub-app input file.
The main app stores the restart data for all sub-apps in its file.
When restarting or recovering, the main app restores the restart data of all sub-apps into MultiApp's *backups*
(a data structure holding all the current state including solution vectors, stateful material properties,
post-processors, restartable quantities declared in objects and etc. of the sub-apps), which are used by
sub-apps to restart/recover the calculations in their initial setups.
The same backups are also used by multiapps for saving/restoring the current state during fixed point iterations.

A sub-app may choose to use a restart file instead of the main backup file by setting [!param](/Problem/FEProblem/force_restart) to true.

!alert warning
[!param](/Problem/FEProblem/force_restart) is experimental.

!syntax list /MultiApps subsystems=False actions=False objects=True

!syntax list /MultiApps subsystems=True actions=False objects=False

!syntax list /MultiApps subsystems=False actions=True objects=False
