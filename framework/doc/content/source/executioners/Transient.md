# Transient

!syntax description /Executioner/Transient

## Normal Usage

The `Transient` Executioner is the primary workhorse Executioner in MOOSE.  Most simulations will use it.

At its most basic the `Transient` Executioner allows a simulation to step through multiple steps in _time_... doing one nonlinear solve per timestep.  Most of the time this type of execution will utilize one or more [`TimeDerivative`](/TimeDerivative.md) Kernels on the variables to solve for their time evolution.

### Primary Parameters

The most important parameters for `Transient` (beyond what [`Steady`](/Steady.md) already provides) are:

- [!param](/Executioner/Transient/dt): The initial timestep size
- [!param](/Executioner/Transient/num_steps): Number of steps to do
- [!param](/Executioner/Transient/end_time): Finish time for the simulation
- [!param](/Executioner/Transient/scheme): The TimeIntegrator to use (see below) - defaults to Implicit/Backward Euler.


See down below for the full list of parameters for this class.

### TimeIntegrators

It's important to note that transient simulations generally use a [TimeIntegrator](TimeIntegrator/index.md).  As mentioned above, there is a `scheme` parameter that is shortcut syntax for selection of that TimeIntegrator.  However, there is also a whole [TimeIntegrator](TimeIntegrator/index.md) system for creating your own or specifying detailed parameters for time integration.

### TimeSteppers

Similarly, the choice of how to move through time (the choice of timestep size) is important as well.  The default [TimeStepper](/TimeStepper/index.md) is [`ConstantDT`](ConstantDT.m) but many other choices can be made using the [TimeStepper](/TimeStepper/index.md) system.

## Load Steps

`Transient` can also be used for simulations that don't necessarily need _time_.  In this context a "transient" calculation can simply be thought of as a series of nonlinear solves.  The time parameter will move forward - but what you do with it, or what it means is up to you.

One good example of this is doing "load steps" for a solid mechanics calculation.  If the only thing that is desired is the final, steady state, solution, but getting to it is extremely difficult, then you might employ "load steps" to slowly ramp up a boundary condition so you can more easily solve from the initial state (the "initial condition") to the final configuration.  In this case you would use "time" as a parameter to control how much of the force is applied (for instance, by using [`FunctionDirichletBC`](/FunctionDirichletBC.md)).

In this case you don't use any [`TimeDerivative`](/TimeDerivative.md) Kernels.  The "transient" behavior comes from changing a condition based on "time".  What that "time" means is up to you to identify (generally, I like to just step through `time = 1,2,3,4..` and define my functions so that at `time = end_steps` the full load is applied.

## Quasi-Transient

Similarly to Load Steps, you can use `Transient` to do "Quasi-Transient" calculations.  This is where some variables are evolving with time derivatives, while others are solved to steady state each step.

A classic example of this is doing coupled thermo-mechanics.  It's very normal for the heat flow to move much more slowly than the solid mechanics. Therefore, classically, it is normal to have a time derivative for your heat conduction equation but none for the solid mechanics so that at each timestep the solid-mechanics is solved to a full steady state based on the current configuration of heat.

This idea works perfectly in MOOSE with `Transient`: just simply only apply [`TimeDerivative`](/TimeDerivative.md) Kernels to the equations you want and leave them off for the others.

## Solving To Steady State

Another use-case is to use `Transient` to solve to a steady state.  In this case there are a few built-in parameters to help detect steady state and stop the solve when it's reached.  You can see them down below in the "Steady State Detection Parameters" section.

It is important to know that you must turn _on_ steady state detection using `steady_state_detection = true` before the other two parameters will do anything.
The parameter `steady_state_tolerance` corresponds to $\tau$ in the following
steady-state convergence criteria:
\begin{equation}
  \frac{\|u^{n+1} - u^n\|}{\Delta t} < \tau \|u\|^{n+1} \,.
\end{equation}

!syntax parameters /Executioner/Transient

!syntax inputs /Executioner/Transient

!syntax children /Executioner/Transient
