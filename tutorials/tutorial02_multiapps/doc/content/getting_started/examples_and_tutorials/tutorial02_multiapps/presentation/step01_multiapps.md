# Step01 MultiApps


!---

## Input File Syntax

`MultiApp` objects are declared in the `[MultiApps]` block and require a `type` just like many other blocks.

The `app_type` is required and is the name of the `MooseApp`
derived app that is going to be executed. Generally, this is the name of the application being
executed, therefore if this parameter is omitted it will default as such. However this system
is designed for running other applications that are compiled or linked into the current app.

A `MultiApp` can be executed at any point during the parent app solve by setting the `execute_on` parameter. The
`positions` parameters is a list of 3D coordinate pairs
describing the offset of the sub-application(s) into the physical space of the parent application.

!listing multiapps/transient_multiapp/dt_from_parent.i block=MultiApps

!---

## Positions id=multiapp-positions

The `positions` parameter is a coordinate offset from
the parent app domain to the sub-app domain, as illustrated below. The parameter
requires the positions to be provided as a set of $(x, y, z)$ coordinates for each sub-app.

The number of coordinate sets determines the actual number of sub-applications created.  If there is
a large number of positions a file can be provided instead using the
`positions_file` parameter.


- The $(x, y, z)$ coordinates are a vector that is being added to the coordinates of the sub-app's
  domain to put that domain in a specific location within the parent app domain.
- If the sub-app's domain starts at $(0,0,0)$ it is easy to think of moving that point around
  using `positions`.
- For sub-apps on completely different scales, `positions` is the point in the parent app domain where
  that app is located.

!---

## MultiApp Position

!media framework/multiapps_positions.png id=multiapps_pos style=width:80%;margin-left:auto;margin-right:auto;
       caption=Example of MultiApp object position.

!---

## Parallel Execution

The `MultiApp` system is designed for efficient parallel execution of hierarchical problems. The
parent application utilizes all processors.  Within each `MultiApp`, all of the processors are split
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

!---

## Basic MultiApp

!row!
!col! width=50%
To get started, let's consider a simple system with two apps as shown on the right.

For now: no `Transfers` so no coupling

Each application will march forward in time together, solve, and output
!col-end!

!col width=50%
!media multiapps_01_hierarchy.png
       style=width:70%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!row-end!

!---

## 01_parent.i

!listing step01_multiapps/01_parent.i

!---

## 01_sub.i

!listing step01_multiapps/01_sub.i

Note how the `sub-app` input file doesn't even "know" it's being run within a MultiApp hierarchy!

!---

## Run 01_parent.i

- Look at the order of execution
- Inspect outputs
- Modify `execute_on` to see what happens

!---

## Sub-App Constraining dt

By default the MultiApp system will "negotiate" a timestep dt that makes sense for the entire hierarchy: it will choose the smallest dt that any app is currently requesting

Let's modify the sub-app to have a smaller timestep and see what happens

!listing step01_multiapps/02_sub_sublimit.i
         caption=02_sub_sublimit.i

!---

## Run 02_parent_sublimit.i

- Note the timestep being used by each app

!media multiapps_02_timesteps.png
       style=width:50%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;


!---

## Subcycling

Forcing all apps to take the same timestep size is very limiting.

Often better to allow the sub-app to take smaller timesteps.  For instance: if the sub-app is a CFD calculation, the timestep size may be limited by numerical criteria or material properties (CFL conditions, etc.).

To allow this: set `sub_cycling=true` in the `MultiApp` block:

!listing step01_multiapps/03_parent_subcycle.i
	 block=MultiApps
         caption=03_parent_subcycle.i

!---

## Run 03_parent_subcycle.i

- Note the timestep size used by each solve
- The sub-app will take however many timesteps are needed to reach the parent app's time
- What happens if the timesteps aren't even?

By default the intermediate steps are NOT output - only the final solution once the sub-app reaches the parent app's time.  To enable outputting all steps solved by the sub-app turn on `output_subcycles` in the MultiApp block.

!---

## Multiple Sub-Apps

Now for a more complicated scenario: multiple sub-apps within the same MultiApp.

This is achieved by giving each sub-app a `position` where that sub-app's domain lies within the parent app's domain.

There are two ways to provide positions:

- `positions`: Space separated x,y,z triplets for the position of each sub-app
- `positions_file`: A filename that includes x,y,z triplets (one per line)

The `positions_file` option is useful if you have MANY sub-apps (for instance: tens of thousands!).

There are two options for specifying input files for the positions:

- A single input file: every sub-app will utilize the same input file
- One input file for each position: every sub-app utilizes a separate input file

!---

## Multiple Sub-App Hierarchy

!listing step01_multiapps/04_parent_multiple.i
	 block=MultiApps
         caption=04_parent_multiple.i

!media multiapps_04_hierarchy.png
       style=width:50%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!---

## Run 04_parent_multiple.i

- Note how there are now three solves when the MultiApp executes
- Note the names of the output files
- Try using the `positions_file` instead
- Try using different input files for each position

Since sub-apps are "offset" into the parent app's domain - the `output_in_position` option can be used to make the output mesh from each sub-app reflect its "true" position within the simulation.  Turn it on and re-visualize the sub-app solutions

!---

## Parallelism

!row!
!col! width=50%
When operating in parallel the MultiApps and sub-apps can be spread across the available processors (MPI-ranks) for faster execution.

The parent app always runs on the full amount of processors.  For this reason, it's often advantageous to make the parent app the largest, most difficult solve.

Each MultiApp executes one-at-a-time (will be clear momentarily).  The sub-apps within a MultiApp are all executed simultaneously (if possible).

To achieve this, the available processors are evenly split among the sub-apps within each MultiApp, as shown on the right.

!col-end!

!col width=50%
!media multiapps_05_parallelism.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!row-end!

!---

## Run 05_parent_parallel.i

- Try 1, 3, 6 MPI procs
- Note the MultiApp Execution time
- Try with `--keep-cout`

Note: to execute in parallel use:

```
mpiexec -n # ./theapp -i input.i
```

where you replace `#` with the number of MPI processes to start.  If you are on a cluster you will need to consult your cluster's documentation for instructions.

!---

## Multiple MultiApps

As discussed before, MultiApps can represent an arbitrary tree of solves.  Often it's the case that one solve may have more than one MultiApp in it.  For instance, a nuclear reactor simulation may need to have solves underneath it for what's happening to the fuel and, separately, what's happening to the fluid.

In parallel, the MultiApps each receive the full amount of processors available from the parent app.  The processors are then split between the sub-apps.  This means that the MultiApps will execute "in-turn" in parallel - one before the other.  The order of executions is automatically determined based on the needs of transfers (more on that in a bit).

To show how this works, we'll execute `06_parent_twoapps.i` which will run a hierarchy like the one below...

!---

## Multiple MultiApps Cont.

!listing step01_multiapps/06_parent_twoapps.i
	 block=MultiApps
         caption=06_parent_twoapps.i

!media multiapps_06_hierarchy.png
       style=width:60%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!---

## Run 06_parent_twoapps.i

- Note how the apps execute
- Run in parallel with 6, 12, 24 procs

!---

## Multiple Levels

The MultiApp hierarchy can also be arbitrarily "deep": that is, any app within the hierarchy can also have it's own MultiApps with more sub-apps, etc.

This allows for arbitrarily deep multi-scale simulation.  Consider a nuclear reactor simulation with seismic analysis:

- Kilometers scale seismic simulation

  - Meters scale containment simulation

    - Meters scale secondary simulation
    - Meters scale pressure vessel simulation

      - Centimeters scale neutronics simulation

        - Centimeters scale fluid simulation

          - Millimeters scale CFD calculation

        - Centimeters scale fuel simulation

          - Micron scale material simulation

It is possible to run this as _one_ calculation with MOOSE MultiApps!

!---

## Run 07_parent_multilevel.i

!listing step01_multiapps/07_parent_multilevel.i
	 block=MultiApps
         caption=07_parent_multilevel.i

!listing step01_multiapps/07_sub_multilevel.i
	 block=MultiApps
         caption=07_sub_multilevel.i
