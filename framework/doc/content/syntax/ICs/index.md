# ICs System

## Description

The `ICs` block within an input file is utilized to define the initial (starting) conditions for
the variables within your simulation. Initial conditions may be applied to both the "unknowns"
(nonlinear or scalar variables) or auxiliary variables. The interface for defining an Initial
Condition is to support a function that returns a value at a "Point", and optionally higher order
derivatives at that point (e.g. Gradient, Second).

## ICs Block

The preferred syntax is to create a top-level "ICs" block with subblocks defining the initial
conditions for one or more variables.

!listing function_ic/parsed_function.i block=ICs

## ICs from an Exodus File

MOOSE contains a shortcut syntax for reading solutions from an Exodus file for the initial
condition from right within the [Variables](Variables/index.html). The name of the variable
and the time step from which to read the solution must be supplied.

!listing from_exodus_solution/nodal_part2.i block=Variables


!syntax list /ICs objects=True actions=False subsystems=False

!syntax list /ICs objects=False actions=False subsystems=True

!syntax list /ICs objects=False actions=True subsystems=False
