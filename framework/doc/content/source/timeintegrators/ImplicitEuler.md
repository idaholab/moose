# ImplicitEuler

This is the default time integrator in MOOSE. It is first order in time, and L-stable, making it
suitable to integrate stiff equation systems. It is also known as the backwards Euler method.

## Description

With $U$ the vector of non linear variables and $A$ a non linear operator
describing the PDE of interest below:

!equation
\dfrac{\partial U(t)}{\partial t} = A(t, U(t))

Using $t+dt$ for the current time step and $t$ for the previous step,
the implicit Euler time integration scheme can be written:

!equation
U(t+dt) = U(t) + dt A(t+dt, U(t+dt))

This is an implicit system with $U(t+dt)$, the variable to solve for, appearing on both sides of the
equation. We solve this system iteratively, usually with a Newton or Newton-Krylov method as described
in the non linear system solve [documentation](systems/NonlinearSystem.md).

!syntax parameters /Executioner/TimeIntegrator/ImplicitEuler

!syntax inputs /Executioner/TimeIntegrator/ImplicitEuler

!syntax children /Executioner/TimeIntegrator/ImplicitEuler
