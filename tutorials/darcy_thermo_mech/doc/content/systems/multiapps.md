# MultiApp System

A system for performing multiple simulations within a main simulation.

!---

MOOSE was originally created to solve fully-coupled systems of [!ac](PDEs), but
not all systems need to be/are fully coupled:

- Multiscale systems are generally loosely coupled between scales
- Systems with both fast and slow physics can be decoupled in time
- Simulations involving input from external codes may be solved

The MultiApp system creates simulations of loosely-coupled systems of fully-coupled equations

!---

## MultiApp Hierarchy

Each "app" is considered to be a solve that is independent, and there is always a "main" that is
driving the simulation

- The "main" app can have any number of `MultiApp` objects
- Each `MultiApp` can represent many sub-applications ("sub-apps")

Each sub-app can solve for different physics from the main application

- A sub-app can be another MOOSE application or could be an external application
- A sub-app can have MultiApps, thus create a multi-level solve

!---

!media darcy_thermo_mech/multiapp_hierarchy.png

!---

## Input File Syntax


MultiApp objects are declared in the `[MultiApps]` block

`app_type`\\
The name of the `MooseApp` derived application to run (e.g., "AnimalApp")

`positions`\\
List of 3D coordinates describing the offset of the sub-application into the physical space of the main application

`execute_on`\\
Allows control when sub-applications are executed: INITIAL, LINEAR, NONLINEAR, TIMESTEP_BEGIN, TIMESTEP_END

`input_files`\\
One input file can be supplied for all the sub-apps or a file can be provided for each

!---

!listing step10_multiapps/problems/step10.i block=MultiApps

!---

## Parallel

The MultiApp system is designed for efficient parallel execution of hierarchical problems.

- The main application utilizes all processors
- The processors are split among each sub-apps within each MultiApp and are run simultaneously
- Multiple MultiApps will be executed one after another
