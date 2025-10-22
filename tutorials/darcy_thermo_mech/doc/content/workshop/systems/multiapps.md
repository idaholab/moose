# [MultiApp System](syntax/MultiApps/index.md)

A system for performing multiple simulations within a main simulation.

!---

MOOSE was originally created to solve fully-coupled systems of [!ac](PDEs), but
not all systems need to be/are fully coupled:

- Multiscale systems are generally loosely coupled between scales
- Systems with both fast and slow physics can be decoupled in time
- Simulations involving input from external codes may be solved

The MultiApp system creates simulations of loosely (or tightly) coupled systems of fully-coupled equations

!---

## Coupling terminology

!row!
!col! width=39%
- Loosely-Coupled

  - Each physics solved with a separate linear/nonlinear solve.
  - Data exchange once per timestep (typically)

- Tightly-Coupled / Picard

  - Each physics solved with a separate linear/nonlinear solve.
  - Data is exchanged and physics re-solved until “convergence”

- Fully-Coupled

  - All physics solved for in one linear/nonlinear solve

!col-end!

!col! width=20%

Example scheme (implicit-explicit)

!equation
\text{solve }M(u_n, v_n) u_{n+1/2} = 0\\
\text{then }N(u_{n+1/2}, v_n) v_{n+1} = 0\\

!equation
\text{solve }M(u_{n,i}, v_{n,i}) u_{n,i+1} = 0\\
\text{then }N(u_{n,i+1}, v_{n,i}) v_{n,i+1} = 0\\
\text{then }M(u_{n,i+1}, v_{n,i+1}) u_{n,i+2} = 0\\
\text{etc }

!equation
\text{solve }\begin{bmatrix}M(u_n, v_n) \\ N(u_n, v_n)\end{bmatrix} \begin{bmatrix}u_n v_n\end{bmatrix} = \begin{bmatrix}0 0\end{bmatrix} \\

!col-end!

!col width=39%
!media images/coupling.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;
       alt=Diagram showing the different levels of coupling possible between apps.

!row-end!

!---

## MultiApp Hierarchy

Each "app" is considered to be a solve that is independent, and there is always a "main" that is
driving the simulation

- The "main" (or "parent") app can have any number of `MultiApp` objects
- Each `MultiApp` can represent many sub-applications ("sub-apps" or "child" apps)

Each sub-app can solve for different physics from the main application

- A sub-app can be another MOOSE application or could be an external application
- A sub-app can have MultiApps, thus create a multi-level solve

!---

!media darcy_thermo_mech/multiapp_hierarchy.png
       alt=Graph showing the multiple levels of nested apps that can make up a MOOSE simulation.

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
