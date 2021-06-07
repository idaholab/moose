# Executioner System

The `Executioner` block describes how the simulation will be executed. It includes commands
to control the solver behavior and time stepping. Time stepping is controlled by a combination
of commands in the `Executioner` block, and the `TimeStepper` block nested within the
`Executioner` block.

The PETSc package is used as the underlying solver in MOOSE, and provides a wide
variety of options to control its behavior. These can be specified in the
Executioner block. Please see the online
[PETSc documentation](http://www.mcs.anl.gov/petsc/documentation/index.html) for
detailed information about these options.

MOOSE provides MultiApp coupling algorithms in all its executioners for tightly-coupled multiphysics simulations.
MultiApps of two groups, those executed before and after the main app, and the main app are solved sequentially within one app coupling iteration.
The execution order of MultiApps within one group is undefined.
Relevant data transfers happen before and after each of the two groups of MultiApps runs.

Because MultiApp allows wrapping another levels of MultiApps, the design enables multi-level app coupling iterations automatically.
App coupling iterations can be relaxed to improve the stability of the convergence.
When a MultiApp is a subapp of a main application and is also the main application for its own subapps, MOOSE allows relaxation of the MultiApp solution
within the main coupling iterations and within the coupling iterations, where the MultiApp is the main application, independently.
More details about the MultiApp coupling algorithms may be found in [FixedPointAlgorithms/index.md])

## Automatic and Default Preconditioning

For most simulations there are two types of solve types that will be used: Newton or Preconditioned
Jacobian Free Newton Krylov (PJFNK). The type is specified using the "solve_type" parameter within the
Executioner block.

Regardless of solve type, NEWTON or PJFNK, preconditioning is an import part of any simulation
(see [Preconditioning/index.md]). By default block diagonal preconditioning is used for all
solves, with two exceptions. If "solve_type" is set to NEWTON or LINEAR and the Preconditioning block is
not defined, single matrix preconditioning (SMP) is used (see [SingleMatrixPreconditioner.md])
with all entries enabled ("full=true"). For NEWTON and LINEAR solves, the auto creation of an SMP objects can be
disabled by setting "auto_preconditioning=false" within the Executioner block (see [CreateExecutionerAction.md]).




!syntax list /Executioner objects=True actions=False subsystems=False

!syntax list /Executioner objects=False actions=False subsystems=True

!syntax list /Executioner objects=False actions=True subsystems=False
