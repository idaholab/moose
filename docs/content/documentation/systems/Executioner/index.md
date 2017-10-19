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

!syntax objects /Executioner

!syntax subsystems /Executioner

!syntax actions /Executioner
