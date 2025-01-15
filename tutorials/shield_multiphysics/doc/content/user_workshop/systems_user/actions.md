# [Action System](source/actions/Action.md)

A system for the programmatic creation of simulation objects and input file syntax.

!---

### Syntax

Action recognize syntax such as `[Kernels]` or `[Outputs]` and perform actions based on them.

!---

<<<<<<< HEAD
## Tasks
=======
## Syntax and Tasks
>>>>>>> a95471b138 (Add intermediary steps back to the simulations)

The MOOSE action system operates on tasks, each task is connected to one or many actions.



!---

## MooseObjectAction

An action designed to build one or many other MooseObjects, such as in the case of the
`[Kernels]`, `[Variables]`, `[BCs]`, etc.. blocks.

!---

## Physics

An action designed to build an entire equation. Solid mechanics, computational fluid dynamics, heat conduction
are all implemented as Physics.

!listing problems/step10.i block=Physics
