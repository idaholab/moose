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

## Variable Ordering

The `addVariables()` method does not directly add the variable to the simulation
but instead caches some data, so that the variable can be added in a later step.
This is done because multiple Components can contribute to the block restriction
of a variable, and the variable should only be added to the problem at the point
at which its full block restriction is known.

The order in which variables are added to the problem may actually be consequential,
since this determines the ordering of equations and unknowns in the resulting
linear and nonlinear systems, which can affect convergence of the solutions.
To allow for a more consistent variable ordering,
[Simulation.md] features a method `setComponentVariableOrder(var, index)`, which
modifies a static ordering map that is used to sort the variables before
adding them to the problem. Here `var` is the variable name to assign an order
index, and `index` is an order index, which may be any `int` value (including
negative values). Each variable name should have a unique index, and it is not
required to have a contiguous set of indices (index gaps are fine). It is also
fine to call this multiple times on the same variable to override the previous
index. This for example can be used to override the ordering of the variables
`rhoA`, `rhouA`, and `rhoEA` in `ThermalHydraulicsApp` from a derived
application. These calls are made in the `registerAll()` method of the application;
for example,

```
void
ThermalHydraulicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  // some registration calls

  Simulation::setComponentVariableOrder("rhoA", 0);
  Simulation::setComponentVariableOrder("rhoEA", 1);
  Simulation::setComponentVariableOrder("rhouA", 2);
}
```

Any variables that do not have an order index are ordered alphabetically after
those variables that do have order indices. For example, in this example, the
set of variables {`c`, `rhoEA`, `a`, `rhouA`, `rhoA`, `b`} would be ordered as
{`rhoA`, `rhoEA`, `rhouA`, `a`, `b`, `c`}.

## Objects and Associated Actions

!syntax list /Components objects=True actions=False subsystems=False

!syntax list /Components objects=False actions=False subsystems=True

!syntax list /Components objects=False actions=True subsystems=False
