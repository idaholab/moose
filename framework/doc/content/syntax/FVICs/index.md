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

## Initial Condition Coupling

Initial Conditions objects in MOOSE can couple to other variables in the system. When this occurs
MOOSE will automatically evaluate dependencies and evaluate initial conditions in an order that
supports the valid inspection of variables when computing initial conditions for other variables.

## Gradients

Some shape function families support gradient degrees of freedom (Hermite). To properly initialize
these DOFs, the initial condition system has an optional override for supplying gradient values.

## Inspecting Current Node or Element Pointers

The initial condition system uses a generic projection algorithm for setting the initial condition
for each supported discritization scheme. In the general case, the projection system may choose
to sample anywhere within the domain and not necessarily right on a mesh node or at an element center
position. However, for common FE discritizations suchs as Lagrange, all of the initial condition
samples are taken at nodes. To support these common cases, InitialCondition derived objects have
access to pointers to both current nodes and current elements. These will be non-null when
samples are taken at the corresponding mesh entity and null otherwise.

## Sanity checks on ICs

- Multiple initial conditions may not be applied to the same variable on the same block
- Multiple initial conditions may not be applied to the same variable on the same boundary
- Global initial conditions will conflict with subdomain or boundary restricted ICs on the same variable

## Block/Boundary Restrictions

The ICs system support both the [BlockRestrictable.md] and
[BoundaryRestrictable.md] interfaces.  When using nodal variables with block
restriction, there is an ambiguity that can occur at inter-block interfaces
where a node sits in elements of two or more different blocks.  To resolve
this ambiguity on multi-block nodes, MOOSE chooses the IC object defined on
the lowest block ID for a node to "win" along the interface; the winning
object's variable *must* be defined on the block - otherwise the IC for the
next lowest block ID for the node is used - and so forth until one has the
variable defined.

!syntax list /ICs objects=True actions=False subsystems=False

!syntax list /ICs objects=False actions=False subsystems=True

!syntax list /ICs objects=False actions=True subsystems=False
