# RigidBodyContactPredictor

!syntax description /Executioner/Predictor/RigidBodyContactPredictor

## Description

MOOSE `Predictor` that warm-starts the monolithic Newton solve by
resolving the contact subproblem --- LM DoFs on the contact sideset,
displacement DoFs within
[!param](/Executioner/Predictor/RigidBodyContactPredictor/k_hops) element
hops into the bulk, and (optionally) the load-control scalar --- with a
small local Newton sub-solve.  The initial guess handed to SNES then
already satisfies the local $(u, \lambda, s)$ coupling; the outer solve
only has to correct the far-field.

On sub-solve failure (non-convergence, KSP breakdown, bound-projection
thrashing) the initial guess is reverted to its entry state and only the
outer SNES runs.  On the shipped force-controlled tests the predictor
typically cuts cumulative nonlinear iterations by three to four and
halves wall time.

The predictor inherits the standard `enable` parameter from `MooseObject`
and declares it *controllable*, so the MOOSE Controls system can flip
the predictor on or off at runtime without touching the input file.
See the [theory page](modules/contact/rigid_contact/theory.md#warm-start-predictor)
for the overall role in the formulation.

## When to use

The sub-solve costs real work of its own (a local Newton with an LU
factorization at every step).  Whether it pays off depends on how much
better its initial guess is than "copy the previous converged state".
Empirically:

- Force-controlled runs benefit strongly.  The load-control scalar
  couples every LM DoF through the reaction integral; without a good
  local guess the outer Newton limit-cycles on the pathological
  $R_s$ direction, so even an expensive local resolution pays back
  several outer iterations per step.  On the shipped 3D
  force-controlled tests the predictor cuts cumulative Newton
  iterations by a factor of three to four and roughly halves wall
  time.
- 2D displacement-controlled runs also typically benefit --- on the
  shipped 2D test the predictor cuts cumulative iterations 3x.
- 3D displacement-controlled runs are usually break-even or slightly
  worse.  "Copy the previous state" is already an excellent warm
  start when the boundary motion is prescribed and the increment
  per step is small; the sub-solve overhead can then exceed the
  outer-iteration savings.

The recommendation is therefore to enable the predictor by default on
force-controlled inputs (as the shipped examples do), and to leave it
off on displacement-controlled inputs unless a specific problem
measures better with it on.  It is legal on either family --- enabling
it on a displacement-controlled input requires no other changes to the
input, since [!param](/Executioner/Predictor/RigidBodyContactPredictor/scalar_variable)
is optional and the sub-solve reduces gracefully to $(u, \lambda)$
alone when no scalar is present.

## Parameters

!syntax parameters /Executioner/Predictor/RigidBodyContactPredictor
