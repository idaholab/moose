# SimplePredictor

!syntax description /Executioner/Predictor/SimplePredictor

The simple predictor uses the solution from the previous time step to update the solution
before a non linear solve.
The simple predictor update is:

!equation
\phi = \phi (1 + s \dfrac{\Delta t}{\Delta t_{old}}) - \phi_{old} s \dfrac{\Delta t}{\Delta t_{old}}

with $\phi$ the solution vector, which includes all the application's non linear variables,
and $s$ a scaling factor, specified by the [!param](/Executioner/Predictor/SimplePredictor/scale)
parameter. That scaling factor is further scaled with the size of the current time step $\Delta t$
divided by the previous one.

## Example input syntax

In this example, a `SimplePredictor` is specified in the executioner to use the previous
time step solution to compute better initial guesses for each non linear solve.

!listing test/tests/predictors/simple/predictor_test.i block=Executioner

!syntax parameters /Executioner/Predictor/SimplePredictor

!syntax inputs /Executioner/Predictor/SimplePredictor

!syntax children /Executioner/Predictor/SimplePredictor
