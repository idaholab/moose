# LinearFVTimeDerivative

## Description

This kernel represents a time derivative term in a partial differential equation
discretized using the finite volume method:

!equation
\int\limits_{V_C} \frac{\partial cu}{\partial t} dV \approx \left(\frac{\partial cu}{\partial t}\right)_C V_C~,

where $\left(\frac{\partial cu}{\partial t}\right)_C$ and $V_C$ are the time derivative of the
field value at the cell center and the cell volume, respectively.
Note that we added a multiplier, $c$ which often represents a material property.
A good example for the multiplier can be the density in the momentum equation
in the Navier Stokes equation.
This can be defined through parameter [!param](/LinearFVKernels/LinearFVTimeDerivative/factor)
that accepts anything that supports functor-based evaluations. For more information on functors in
MOOSE, see [Functors/index.md].
This kernel adds to the matrix diagonal and right hand side of a
linear system and the contributions depend on the
method chosen for time integration. For more information on available methods, see
the [TimeIntegrators](Executioner/TimeIntegrators/index.md) page.
For example, with an implicit Euler scheme the contribution to the right hand side becomes:

!equation
\frac{c_C}{\Delta t}V_C,

where $\Delta t$ and $c_C$ are the time step size and multiplier at the cell center,
respectively. With these, the contribution to the right hand side becomes:

!equation
\frac{c_C u_{old,C}}{\Delta t}V_C,

where $u_{old,C}$ represents the solution at the previous time step.

## Example Syntax

The case below demonstrates the use of `LinearFVTimeDerivative` used in a simple
linear time-dependent diffusion problem:

!listing test/tests/time_integrators/implicit-euler/ie-linearfv.i block=LinearFVKernels

!syntax parameters /LinearFVKernels/LinearFVTimeDerivative

!syntax inputs /LinearFVKernels/LinearFVTimeDerivative

!syntax children /LinearFVKernels/LinearFVTimeDerivative
