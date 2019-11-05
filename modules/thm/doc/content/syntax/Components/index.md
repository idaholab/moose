# Components System

The Components system is used to assemble the pieces in a systems calculation.
For example, it can be used to put together a network of pipes and 0-D models
in a thermal hydraulics system. Components perform a number of tasks, including
the following:

- Adding mesh(es),
- Adding variables,
- Adding initial conditions, and
- Adding equations (kernels, boundary conditions, constraints, etc.).

Thus, applications using this system typically do not require the user to create
any of the corresponding blocks in their input files.

!syntax list /Components objects=True actions=False subsystems=False

!syntax list /Components objects=False actions=False subsystems=True

!syntax list /Components objects=False actions=True subsystems=False
