# ImplicitEuler

This is the default time integrator in MOOSE. It is first-order in time, and L-stable, making it
suitable to integrate stiff equation systems. It is also known as the backwards Euler method.

## Description

With $U$, the vector of nonlinear variables, and $A$, a nonlinear operator,
we write the PDE of interest as:

!equation
\dfrac{\partial U}{\partial t} = A(t, U(t))

Using $t+\Delta t$ for the current time step, and $t$ for the previous step,
the implicit Euler time integration scheme can be written:

!equation
U(t+\Delta t) = U(t) + \Delta t A(t+\Delta t, U(t+\Delta t))

This is an implicit system with $U(t+\Delta t)$, the variable to solve for, appearing on both sides of the
equation. We solve this system iteratively, usually with a Newton or Newton-Krylov method as described
in the non linear system solve [documentation](systems/NonlinearSystem.md).

## Contributions to linear systems

For [linear systems](systems/LinearSystem.md), on top of creating the
time derivatives of the degrees of freedom, this provides contributions
to the matrix diagonal and the right hand side. Taking a finite volume system for example,
the contributions to the matrix diagonal will be:

!equation
\frac{1}{\Delta t}V_C,

where $\Delta t$ and $\frac{V_C}$ are the time step size and cell volume,
respectively. The contribution to the right hand side is:

!equation
\frac{u_{old,C}}{\Delta t}V_C,

!syntax parameters /Executioner/TimeIntegrator/ImplicitEuler

!syntax inputs /Executioner/TimeIntegrator/ImplicitEuler

!syntax children /Executioner/TimeIntegrator/ImplicitEuler
