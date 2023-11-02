# FVICs System

## Description

The `FVICs` block within an input file is used to define the initial (starting) conditions for
the finite volume variables within your simulation. Initial conditions may be applied to both the "unknowns"
(nonlinear) or auxiliary variables.
It computes the values of the variable at the cell centroids.

## FVICs Block

The preferred syntax is to create a top-level "FVICs" block with subblocks defining the initial
conditions for one or more variables.

!listing fvics/function_ic/parsed_function.i block=FVICs

## FVICs from an Exodus File

MOOSE contains a shortcut syntax for reading solutions from an Exodus file for the initial
condition from right within the [Variables](Variables/index.html). The name of the variable and the time step from which to read the solution must be supplied.

!listing fvics/file_ic/file_restart.i block=Variables

## Sanity checks on FVICs

- Multiple FV initial conditions may not be applied to the same variable on the same block
- Global initial conditions will conflict with subdomain or boundary restricted ICs on the same variable

!syntax list /FVICs objects=True actions=False subsystems=False

!syntax list /FVICs objects=False actions=False subsystems=True

!syntax list /FVICs objects=False actions=True subsystems=False
