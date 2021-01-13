# Step01 MultiApps


!---

## Input File Syntax

`MultiApp` objects are declared in the `[MultiApps]` block and require a `type` just like may other blocks.

The `app_type` is required and is the name of the `MooseApp`
derived app that is going to be executed. Generally, this is the name of the application being
executed, therefore if this parameter is omitted it will default as such. However this system
is designed for running other applications that are compiled or linked into the current app.

A `MultiApp` can be executed at any point during the master solve by setting the `execute_on` parameter. The
`positions` parameters is a list of 3D coordinate pairs
describing the offset of the sub-application(s) into the physical space of the master application.

!listing multiapps/transient_multiapp/dt_from_master.i block=MultiApps

!---

## Positions id=multiapp-positions

The `positions` parameter is a coordinate offset from
the master app domain to the sub-app domain, as illustrated below. The parameter
requires the positions to be provided as a set of $(x, y, z)$ coordinates for each sub-app.

The number of coordinate sets determines the actual number of sub-applications created.  If there is
a large number of positions a file can be provided instead using the
`positions_file` parameter.


- The $(x, y, z)$ coordinates are a vector that is being added to the coordinates of the sub-app's
  domain to put that domain in a specific location within the master domain.
- If the sub-app's domain starts at $(0,0,0)$ it is easy to think of moving that point around
  using `positions`.
- For sub-apps on completely different scales, `positions` is the point in the master domain where
  that app is located.

!---

## MultiApp Position

!media framework/multiapps_positions.png id=multiapps_pos style=width:80%;margin-left:auto;margin-right:auto;
       caption=Example of MultiApp object position.

!---

## Parallel Execution

The `MultiApp` system is designed for efficient parallel execution of hierarchical problems. The
master application utilizes all processors.  Within each `MultiApp`, all of the processors are split
among the sub-apps. If there are more sub-apps than processors, each processor will solve for
multiple sub-apps.  All sub-apps of a given `MultiApp` are run simultaneously in parallel. Multiple
`MultiApps` will be executed one after another.

!---

## Dynamically Loading Multiapps

If building with dynamic libraries (the default) other applications can be loaded without adding them
to your Makefile and registering them. Simply set the proper `type` in your input file
(e.g. `AnimalApp`) and MOOSE will attempt to find the other library dynamically.

- The path (relative preferred) can be set in your input file using the parameter
  `library_path`; this path needs to point to the `lib` folder
  within an application directory.
- The `MOOSE_LIBRARY_PATH` may also be set to include paths for MOOSE to search.

Each application must be compiled separately since the main application Makefile does not have
knowledge of any sub-app application dependencies.
