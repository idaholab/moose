<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# MultiApps System

- MOOSE was originally created to solve fully-coupled systems of PDEs.
- Not all systems need to be/are fully coupled:

  - Multiscale systems are generally loosely coupled between scales.
  - Systems with both fast and slow physics can be decoupled in time.
  - Simulations involving input from external codes might be solved somewhat decoupled.

- To MOOSE these situations look like loosely-coupled systems of fully-coupled equations.
- A `MultiApp` allows you to simultaneously solve for individual physics systems.
- Each "App" is considered to be a solve that is independent.
- There is always a "master" App that is doing the "main" solve.
- A "master" App can then have any number of `MultiApps`.
- Each `MultiApp` can represent many (hence Multi!) "sun-apps".
- The sub-apps can be solving for completely different physics from the main application.
- They can be other than MOOSE applications, or might represent external applications.
- A sub-app can, itself, have `MultiApps`... leading to multi-level solves.

!media large_media/multi_apps/multiapp_hierarchy.png
       caption=MultiApp Hierarchy
       style=width:50%;

## Input File Syntax

- `MultiApps` are declared in the `MultiApps` block.
- They require a `type` just like any other block.
- `app_type` is required and is the name of the `MooseApp` derived App that is going to be run. Generally this is something like `AnimalApp`.
- A `MultiApp` can be executed at any point during the master solve. You get that using `execute_on` to one of: `initial`, `residual`, `jacobian`, `timestep_begin`, or `timestep`.
- `positions` is a list of 3D coordinate pairs describing the offset of the sub-applcation into the physical space f the master application. More on this in a moment.
- You can either provide one input file for all the sub-apps... or provide one per position.

```puppet
[MultiApps]
  [./some_multi]
    type = TransientMultiApp
    app_type = SomeApp
    execute_on = timestep_end
    positions = '0.0 0.0 0.0
                 0.5 0.5 0.0
                 0.6 0.6 0.0
                 0.7 0.7 0.0'
   input_files = 'sub.i'
 [../]
[]
```

## Dynamically Loading MultiApps

- If you are buiding with dynamic libraries (the default) you may load other applicationswithout explicitly adding them to your Makefile and registering them.
- Simply set the proper `Type` in your input file (e.g. `AnimalApp`) and MOOSE attempt to find the other library dynamically.
- You may specify a path (relative preferred) in your input file using the parameter `library_path`. This path needs to point to the `lib` folder underneath your application.
- You may also set and environment variable for paths to search: `MOOSE_LIBRARY_PATH`.
- Note: You will need to compile each application separately since the Makefile does not have any knowledge of the dependent application.

## TransientMultiApp

- Some of the currently-available MultiApp are `TransientMultiApp` and `FullSolveMultiApp`.
- A `TransientMultiApp` requires that you "sub-apps" use an `Executioner` derived from `Transient`.
- A `TransientMultiApp` will be taken into account during time step selection inside the "master" `Transient` executioner.
- By default, the minimum `dt` over the master and all sub-apps is used.
- However, we have the ability to do "sub-cycling", which allows a sub-app to take multiple time steps during a single master app time step.

## Positions

- The position parameter allows you to define a "coordinate offset" of the sub-app's coordinates into the master app's domain.
- You must provide one set of $(x,y,z)$ coordinates for each sub-app.
- The number of coordinate sets determines the actual number of sub-applications.
- If you have a large number of positions you can read them from a file using `positions_file = filename`.
- You can think of the $(x,y,z)$ coordinates as a vector that is being added to the coordinates of your sub-app's domain to put that domain in a specific spot within the master domain.
- If your sub-app's domain starts at $(0,0,0)$ it is easy to think of moving that point around using `positions`.
- For sub-apps on completely different scales, `positions` is the point in the master domain where the App is.

!media large_media/multi_apps/position.png
       style=width:50%;


## Parallel

!media large_media/multi_apps/multiapp_hierarchy.png
       caption=MultiApp Hierarchy
       style=width:50%;

- The `MultiApp` system is designed for efficient parallel execution of hierarchical problems.
- The master application utilizes all processors.
- Within each `MultiApp`, all of the processors are split among the sub-apps.
- If there are more sub-apps than processors, each processor will solve for multiple sub-apps.
- All sub-apps of a given `MultiApp` are simultaneously in parallel.
- Multiple `MultiApps` will be executed one after another.

## Further MultiApp Documentation

!syntax list /MultiApps objects=True actions=False subsystems=False

!syntax list /MultiApps objects=False actions=False subsystems=True

!syntax list /MultiApps objects=False actions=True subsystems=False