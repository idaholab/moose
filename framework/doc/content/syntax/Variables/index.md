# Variables System

## Description

The `Variables` block within an input file is utilized to define the unknowns within a system
of partial differential equations. These unknowns are often referred to as nonlinear variables
within documentation. The nonlinear variables defined within this block are used by
[Kernel objects](syntax/Kernels/index.md) to define the equations for a simulation.

## Example

The following input file snippet demonstrates the creation of a single nonlinear variable that
is used by two Kernel objects, refer to [ex02_kernel.md] for more details.

!listing ex02_kernel/ex02.i block=Variables Kernels


!syntax parameters /Variables/AddVariableAction

!syntax list /Variables objects=True actions=False subsystems=False

!syntax list /Variables objects=False actions=False subsystems=True

!syntax list /Variables objects=False actions=True subsystems=False
