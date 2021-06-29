# Action System

A system for the programmatic creation of simulation objects and input file syntax.

!---

## Creating an Action

Inherit from `Action` or `MooseObjectAction` and override the `act()` method.

Notice, the constructor uses a copy of an `InputParameters` object. This is by design to allow the
parameters to be manipulated and re-used during object creation.

!---

## Action Object

An action design to build specific objects, such as the case in [#step09] for tensor mechanics.

!listing problems/step9.i block=Modules/TensorMechanics/Master

!---


## Syntax and Tasks

The MOOSE action system operates on tasks, each task is connected to one or many actions.

For each task the `act()` method is called for each task, thus the act method can be used to
create any number of objects.

!---

In general, the following macros are called within an application `registerAll` method to
create the necessary syntax and tasks to build the desired objects.

`registerSyntax(action, action_syntax)`\\
Creates input file syntax provided in "action_syntax" and associates with the "action" provided

`registerSyntaxTask(action, action_syntax, task)`\\
Same as above, but also creates a named task that will be executed.

`registerTask(name, is_required)`\\
Creates a named task that actions can be associated.

`addTaskDependency(action, depends_on)`\\
Create a dependency between two tasks to help control the order of operation of task execution

!---

## MooseObjectAction Object

An action designed to build one or many other MooseObjects, such as in the case of the
`[Dampers]` block.

!---

## AddDamperAction.h

!listing AddDamperAction.h

!---

## AddDamperAction.C

!listing AddDamperAction.C

!---

## Moose.C

!listing base/Moose.C line=Dampers/*
