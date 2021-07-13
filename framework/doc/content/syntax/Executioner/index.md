# Executioner System

The `Executioner` block describes how the simulation will be executed. It includes commands
to control the solver behavior.
There are three basic executioners: [Steady](Steady.md), [Transient](Transient.md) and [Eigenvalue](Eigenvalue.md).

!alert note
The executioners [InversePowerMethod](InversePowerMethod.md) and [NonlinearEigen](NonlinearEigen.md) are two executioners for solving nonlinear generalized
eigenvalue problems implemented previously in MOOSE. We pushed the PJFNK-based algorithm into [SLEPc](https://slepc.upv.es/) and added [Eigenvalue](Eigenvalue.md)
to interact with SLEPc. The new implementation contains the capabilities provided by these two eigenvalue executioners and all other solving
options in SLEPc. Users are enouraged to use [Eigenvalue](Eigenvalue.md).

Time stepping is controlled by a combination of commands in the `Executioner` block, and the `TimeStepper` block nested within the
`Executioner` block. More discussions on time stepping can be found at [TimeStepper](TimeStepper/index.md).
Various time integration schemes including forward/backward Euler, Crank-Nicolson, etc. are available, please refer to [TimeIntegrator](TimeIntegrator/index.md).

The PETSc package is used as the underlying solver in MOOSE, and provides a wide
variety of options to control its behavior. These can be specified in the
Executioner block. Please see the online
[PETSc documentation](http://www.mcs.anl.gov/petsc/documentation/index.html) for
detailed information about these options.

MOOSE provides MultiApp coupling algorithms in all its executioners for tightly-coupled multiphysics simulations.
MultiApps of two groups, those executed before and after the main app, and the main app are solved sequentially within one app coupling iteration.
The execution order of MultiApps within one group is undefined and irrelevant because there is no direct data transfer among sub-apps.
Relevant data transfers happen before and after each of the two groups of MultiApps runs from/to main application to/from the sub-apps.

Because MultiApp allows wrapping another levels of MultiApps, the design enables multi-level app coupling iterations automatically.
App coupling iterations can be relaxed to improve the stability of the convergence.
When a MultiApp is a subapp of a main application and is also the main application for its own subapps, MOOSE allows relaxation of the MultiApp solution
within the main coupling iterations and within the coupling iterations, where the MultiApp is the main application, independently.
More details about the MultiApp coupling algorithms may be found in [FixedPointAlgorithms/index.md])

## Automatic and Default Preconditioning

For most simulations with PETSc there are two types of solve types that will be used: Newton or Preconditioned
Jacobian Free Newton Krylov (PJFNK). The type is specified using the "solve_type" parameter within the
Executioner block.

Regardless of solve type, NEWTON or PJFNK, preconditioning is an import part of any simulation
(see [Preconditioning/index.md]). By default block diagonal preconditioning is used for all
solves, with two exceptions. If "solve_type" is set to NEWTON or LINEAR and the Preconditioning block is
not defined, single matrix preconditioning (SMP) is used (see [SingleMatrixPreconditioner.md])
with all entries enabled ("full=true"). For NEWTON and LINEAR solves, the auto creation of an SMP objects can be
disabled by setting "auto_preconditioning=false" within the Executioner block (see [CreateExecutionerAction.md]).

## Modular design

Executioner system is designed with a modular approach.
Each indiviual module is referred as a solve object.
For example, the solver utilizing PETSc package or SLEPc package are managed in [FEProblemSolve](FEProblemSolve.md),
which takes parameters such as [!param](/Executioner/FEProblemSolve/solve_type), [!param](/Executioner/FEProblemSolve/petsc_options_iname), [!param](/Executioner/FEProblemSolve/nl_rel_tol), etc.
A list of available solve objects is in the following:

| Type          | Functionality           | Output |
| :- | :- | :- |
| SolveObject         | Empty operation | No |
| FEProblemSolve      | Use PETSc/SLEPc to solve a MOOSE nonlinar strongly-coupled multiphysics system | No |
| PicardSolve         | Perform Picard iterations often with MultiApps for tightly-coupled multiphysics systems | timestep_begin |
| SecantSolve         | Perform Secant iterations often with MultiApps for tightly-coupled multiphysics systems | timestep_begin |
| SteffensenSolve     | Perform Steffensen iterations often with MultiApps for tightly-coupled multiphysics systems | timestep_begin |

Each solve object can have an inner solve object, which constitutes a solve step of the nesting solve object.
[FEProblemSolve](FEProblemSolve.md) in this sense is special because it does not allow an inner solve object.
Solve objects together can form a compound solve object if desired.
Solve objects can be added through an input syntax with sub-blocks of the *Executioner* block like materials with sub-blocks of the *Materials* block.

MOOSE supports two ways of using these solve objects: one is with the previously mentioned three prescribed executioners.
Each executioner provides a fixed set of functionalities.
For example, [Steady](Steady.md) is made to support mesh adaptation, interacting with PETSc, and performing iterations for tightly coupled multiphysics calculations.
Another way is to come up custom executioners that integrate existing MOOSE solve objects or new solve objects seamlessly for more complicated simulation tasks.

!syntax list /Executioner objects=True actions=False subsystems=False

!syntax list /Executioner objects=False actions=False subsystems=True

!syntax list /Executioner objects=False actions=True subsystems=False
