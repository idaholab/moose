# Predictor System

Predictors, as the name suggests, predict the future solution based on the previous
solutions. They, currently, act on the entire nonlinear solution vector. They are
applied right before the beginning of the nonlinear equations solve.

A predictor is an optimization to apply on top of time integration schemes, or may
already be part of some explicit or multi-step schemes.
As a predictor extrapolates based on earlier solutions, it will not be able to
predict changes due to threshold phenomena, and fares best when the evolution of
the variables is smooth.

!alert note
Using a predictor in an implicit time integration scheme will only update the starting
guess for the next nonlinear iteration solve. It should not modify the result, unless
additional intricacies such as a predictor-corrector scheme is used.

!alert note
Using a predictor changes the value of the solution on each time step. As such,
it can lower the accuracy of explicit higher order time integration scheme if it does not
work tightly within that scheme.

!syntax list /Executioner/Predictor objects=True actions=False subsystems=False

!syntax list /Executioner/Predictor objects=False actions=False subsystems=True

!syntax list /Executioner/Predictor objects=False actions=True subsystems=False
