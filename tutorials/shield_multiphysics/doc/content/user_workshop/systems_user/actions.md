# [Action System](source/actions/Action.md)

A system for the programmatic creation of simulation objects and input file syntax.

!---

### Syntax

Action recognize syntax such as `[Kernels]` or `[Outputs]` and perform actions based on them.
Many actions simply create an object, using the `type` parameter. Other parse parameters
in the block and perform the relevant setup steps.

!---

## Tasks

The MOOSE action system operates on tasks, each task is connected to one or many actions.
Tasks are ordered based on their dependencies, which are set by the developers. This ensures
the correctness of the simulation setup.
To see tasks being executed, use `Debug/show_actions=true`.

!---

## MooseObjectAction

An action designed to build one or many other MooseObjects, such as in the case of the
`[Kernels]`, `[Variables]`, `[BCs]`, etc.. blocks.

!---

## Physics

An action designed to build an entire equation. Solid mechanics, computational fluid dynamics, heat conduction
are all implemented as Physics. Depending on the `Physics`, it can include only the kernels, or also include
boundary conditions and material properties.

!listing problems/step10.i block=Physics

!---

`Physics` are meant to facilitate numerous simulation common needs such as:

- multi-system simulation setups
- preconditioning
- initialization from mesh files
- interaction with other `Physics`, creating the adequate coupling terms
