# Dampers System

Dampers are used to decrease the attempted change to the solution with each nonlinear step.
This can be useful in preventing the solver from changing the solution dramatically from one
step to the next. This may prevent, for example, the solver from attempting to evaluate negative
temperatures.

## Damping the predictor id=predictor

By default a damper limits the updates of the nonlinear iterations. With `apply_to`, a damper can
instead (or also) limit the change of the initial guess made by a
[Predictor](syntax/Executioner/Predictor/index.md) at the start of a time step. A predictor
that extrapolates too far can produce an initial guess the solve cannot start from, for example a
mesh with inverted elements. A damper on the predictor is evaluated at the predicted solution and
scales the change back by its damping factor. As on the nonlinear updates, the change includes the
degrees of freedom of preset Dirichlet boundary conditions, which are set after the damping. The
dampers of the two stages are independent, so the predictor and the nonlinear iterations can be
limited by different dampers or settings:

!listing test/tests/predictors/simple/predictor_damped_test.i block=Dampers

When a damper on the predictor fails, for example because its damping is below `min_damping`, the
predictor's `damping_failure` parameter decides whether the prediction is skipped for that time
step (the default) or the time step is cut.

!syntax list /Dampers objects=True actions=False subsystems=False

!syntax list /Dampers objects=False actions=False subsystems=True

!syntax list /Dampers objects=False actions=True subsystems=False
