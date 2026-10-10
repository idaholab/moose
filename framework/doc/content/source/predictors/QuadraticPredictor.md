# QuadraticPredictor

!syntax description /Executioner/Predictor/QuadraticPredictor

## Description

`QuadraticPredictor` uses a second-order extrapolation of previous nonlinear
solution vectors to provide the initial guess for a transient solve. With
[!param](/Executioner/Predictor/QuadraticPredictor/scale) set to 1, the full
quadratic prediction is applied. With [!param](/Executioner/Predictor/QuadraticPredictor/scale)
set to 0, the predictor leaves the current initial guess unchanged.

For nonlinear problems with quasistatic load increments, the predictor may
provide a better Newton starting point; depending on the problem, this can allow
larger load increments and/or fewer Newton iterations without changing the
accepted implicit solution.

The prediction is exact for solution histories that are quadratic in time once
enough old solutions and time-step sizes are available. For nonuniform time
steps, the extrapolation uses the corresponding solution times rather than
assuming a constant time-step size.

See also [SimplePredictor.md] for linear extrapolation and [AdamsPredictor.md]
for another second-order predictor.

!alert warning title=Robustness on strongly nonlinear problems
At uniform time step the coefficients of $u^n$, $u^{n-1}$, and
$u^{n-2}$ are $(3, -3, 1)$, which can amplify model nonlinearity in the
initial Newton iterate. For strongly nonlinear problems, including finite
strain solid mechanics, full-scale quadratic prediction can drive Newton into
a non-physical region of state space; one typical symptom is
`DIVERGED_FUNCTION_NANORINF`. Consider [AdamsPredictor.md], which has less
amplifying uniform time-step coefficients $(5/2, -2, 1/2)$, reduce
[!param](/Executioner/Predictor/QuadraticPredictor/scale), or add a nonlinear line search when the
full quadratic predictor is too aggressive.

The predicted solution is computed from the three most recent accepted solution
vectors:

!equation
\phi_{pred} = A \phi + B \phi_{old} + C \phi_{older}

with

!equation
A = \frac{(\Delta t + \Delta t_{old})(\Delta t + \Delta t_{old} + \Delta t_{older})}
         {\Delta t_{old}(\Delta t_{old} + \Delta t_{older})}

!equation
B = -\frac{\Delta t(\Delta t + \Delta t_{old} + \Delta t_{older})}
          {\Delta t_{old}\Delta t_{older}}

!equation
C = \frac{\Delta t(\Delta t + \Delta t_{old})}
         {\Delta t_{older}(\Delta t_{old} + \Delta t_{older})}

The [!param](/Executioner/Predictor/QuadraticPredictor/scale) parameter damps the
update from the current accepted solution to this full quadratic extrapolation.

## Example input syntax

In this example, a `QuadraticPredictor` is specified in the executioner to
predict a diffusion solution driven by a quadratic-in-time boundary condition.

!listing test/tests/predictors/quadratic_predictor/quadratic_predictor.i block=Executioner

!syntax parameters /Executioner/Predictor/QuadraticPredictor

!syntax inputs /Executioner/Predictor/QuadraticPredictor

!syntax children /Executioner/Predictor/QuadraticPredictor
