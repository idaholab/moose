# AdamsPredictor

!syntax description /Executioner/Predictor/AdamsPredictor

An Adams predictor is automatically added by MOOSE for the [AB2PredictorCorrector.md]
time stepping/integration scheme.

The formula for the update by Adams' predictor is:

!equation
\phi_{new} = A \phi + B \phi_{old} + C \phi_{older}

with \phi being the solution vector, including all degrees of freedom for the nonlinear variables
and

!equation
A = 1 + \dfrac{\Delta t}{\Delta t_{old}} (1 + \dfrac{\Delta t}{2\Delta t_{old}})

!equation
B = - \dfrac{\Delta t}{\Delta t_{old}} (1 + \dfrac{\Delta t}{2\Delta t_{old}} + \dfrac{\Delta t}{2\Delta t_{older}})

!equation
C = \dfrac{\Delta t}{\Delta t_{old}} \dfrac{\Delta t}{2\Delta t_{older}}

with $\dfrac{\Delta t}$, $\dfrac{\Delta t_{old}}$ and $\dfrac{\Delta t_{older}}$
being the current, previous and antepenultimate time steps sizes.

## Example input syntax

In this example, an `AdamsPredictor` is implicitly being created by specifying the
`AB2PredictorCorrector` time stepping scheme. The predictor is being used on every
time step to perform a first update step.

!listing test/tests/time_integrators/aee/aee.i block=Executioner

!syntax parameters /Executioner/Predictor/AdamsPredictor

!syntax inputs /Executioner/Predictor/AdamsPredictor

!syntax children /Executioner/Predictor/AdamsPredictor
