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

MOOSE provides Picard iterations in all its executioners for tightly-coupled multiphysics simulations.
MultiApps of two groups of before and after master app and master app are solved sequentially within one Picard iteration.
The execution order of MutlApps within one group is undefined.
Relevant data transfers happen before and after each of the two groups of MultiApps runs.
Because MultiApp allows wrapping another levels of MultiApps, the design enables multi-level Picard iterations automatically.
Picard iterations can be relaxed to improve the stabilitity of the convergence.
When a MultiApp is a subapp of a master and a master of its own subapps, MOOSE allows relaxation of the MultiApp solution
within the master Picard iterations and within the Picard iterations, where the MultiApp is the master, independently.

!syntax list /Executioner objects=True actions=False subsystems=False

!syntax list /Executioner objects=False actions=False subsystems=True

!syntax list /Executioner objects=False actions=True subsystems=False
