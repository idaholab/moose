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

## Implementation

The following methods are available to implement for a component, in the order
in which they are called:

- `preSetupMesh()`: This performs any setup that needs to occur before setting
  up the mesh that is not safe to perform in the constructor.
- `setupMesh()`: This performs any mesh setup such as creating meshes or naming mesh sets.
- `init()`: This performs any initialization steps that are not safe to perform
  in the constructor. This typically used when other component(s) need to have
  been constructed first.
- `initSecondary()`: This performs secondary initialization step(s) that require
  `init()` to have been called on other component(s).
- `check()`: This performs checks on the component, logging warnings and errors.
- `addVariables()`: This is used to add variables to the simulation.
- `addMooseObjects()`: This is used to add MOOSE objects to the simulation.

If a component's methods should be called after those of another, then the
first component can add the second, called `other_component` here, as a dependency:

```
addDependency("other_component");
```

This should be done in the constructor.

## Objects and Associated Actions

!syntax list /Components objects=True actions=False subsystems=False

!syntax list /Components objects=False actions=False subsystems=True

!syntax list /Components objects=False actions=True subsystems=False
