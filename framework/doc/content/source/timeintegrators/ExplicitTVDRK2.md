# ExplicitTVDRK2

!syntax description /Executioner/TimeIntegrator/ExplicitTVDRK2

## Description

The method [!cite](gottlieb1998) consists of two stages:

Stage 1.

!equation
R_{NL} = M(U^{(1)}-U^n)/\Delta t - F(t^n,U^n)

!equation
t^{(1)} = t^{n} + \Delta t = t^{n+1}

Stage 2.

!equation
R_{NL} = M(2U^{(2)}-U^{(1)}-U^n)/(2\Delta t) - (1/2)F(t^{(1)},U^{(1)})

!equation
U^{n+1} = U^{(2)}

The method requires two mass matrix (linear) system solves
per timestep. Although strictly speaking these are "two stage"
methods, we treat the "update" step as a third stage, since in
finite element analysis the update step requires a mass matrix
solve.

!alert warning
To use the explicit TimeIntegrators derived from this
method, you must generally add "implicit=false" to the Kernels,
Materials, etc. used in your simulation, so that MOOSE evaluates
them correctly!  An important exception are TimeDerivative kernels,
which should never be marked "implicit=false".

!syntax parameters /Executioner/TimeIntegrator/ExplicitTVDRK2

!syntax inputs /Executioner/TimeIntegrator/ExplicitTVDRK2

!syntax children /Executioner/TimeIntegrator/ExplicitTVDRK2
