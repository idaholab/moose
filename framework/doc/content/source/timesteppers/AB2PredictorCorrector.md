# AB2PredictorCorrector

!syntax description /Executioner/TimeStepper/AB2PredictorCorrector

This time stepper first adds an [AdamsPredictor.md] to the problem. The predictor
uses previous solutions to compute a predicted solution vector. This prediction is
then compared using a $L\_infty$ norm to the solution. If the error is lower than
[!param](/Executioner/TimeStepper/AB2PredictorCorrector/e_max),
then the time step is accepted. If not, then it is reduced.

The time step is regularly increased based on the
[!param](/Executioner/TimeStepper/AB2PredictorCorrector/steps_between_increase) parameter. The
magnitude of the increase is based on the magnitude of the prediction error.

The `AB2PredictorCorrector` may be used with the following three time integration schemes:
implicit Euler (default in MOOSE), Crank Nicholson and 2nd order backward differences (BDF2).

## Example input syntax

In this example, we solve a simple heating problem with backwards differences and a predictor
corrector scheme. The prediction and correction steps can be observed during the solve.

!listing test/tests/time_integrators/aee/aee.i block=Executioner

!syntax parameters /Executioner/TimeStepper/AB2PredictorCorrector

!syntax inputs /Executioner/TimeStepper/AB2PredictorCorrector

!syntax children /Executioner/TimeStepper/AB2PredictorCorrector
