# IterationAdaptiveDT

!syntax description /Executioner/TimeStepper/IterationAdaptiveDT

## Description

The `IterationAdaptiveDT` Time Stepper provides a means to adapt the time step
size based on the difficulty of the solution.

`IterationAdaptiveDT` grows or shrinks the time step based on the number of iterations taken
to obtain a converged solution in the last converged step. The required `optimal_iterations`
parameter controls the number of nonlinear iterations per time step that provides optimal solution
efficiency. If more iterations than that are required to obtain a converged solution, the time step
may be too large, resulting in undue solution difficulty, while if fewer iterations are required, it
may be possible to take larger time steps to obtain a solution more quickly.

A second parameter, `iteration_window`, is used to control the size of the region in which
the time step is held constant. As shown in [fig:adaptive_dt_criteria], if the number of nonlinear iterations
for convergence is lower than (`optimal_iterations-iteration_window`), the time step is
increased, while if more than (`optimal_iterations+iteration_window`), iterations are required,
the time step is decreased. The `iteration_window` parameter is optional. If it is not
specified, it defaults to 1/5 the value specified for `optimal_iterations`.

The decision on whether to grow or shrink the time step is based both on the number of nonlinear
iterations and the number of linear iterations. The parameters mentioned above are used to
control the optimal iterations and window for nonlinear iterations. The same criterion is applied
to the linear iterations. Another parameter, `linear_iteration` ratio, which defaults to 25, is
used to control the optimal iterations and window for the linear iterations. These are calculated
by multiplying `linear_iteration_ratio` by optimal iterations and iteration window,
respectively.

To grow the time step, the growth criterion must be met for both the linear iterations and nonlinear
iterations. If the time step shrinkage criterion is reached for either the linear or nonlinear
iterations, the time step is decreased. To control the time step size only based on the number of
nonlinear iterations, set `linear_iteration_ratio` to a large number.

If the time step is to be increased or decreased, that is done using the factors specified with the
`growth_factor` and `cutback_factor`, respectively. If a solution fails to converge when adaptive
time stepping is active, a new attempt is made using a smaller time step in the same manner
as with the fixed time step methods. The maximum and minimum time steps can be optionally
specified in the `Executioner` block using the `dtmax` and `dtmin` parameters, respectively.

In addition to controlling the time step based on the iteration count, `IterationAdaptiveDT`
also has an option to limit the time step based on the behavior of a time-dependent function,
optionall specified by providing the function name in `timestep_limiting_function`. This
is typically a function that is used to drive boundary conditions of the model. The step is
cut back if the change in the function from the previous step exceeds the value specified in
`max_function_change`. This allows the step size to be changed to limit the change in the boundary
conditions applied to the model over a step. In addition to that limit, the boolean parameter
`force_step_every_function_point` can be set to `true` to force a time step at every point in a
`Piecewise` function. The time step size post function sync can be reset via the [!param](/Executioner/TimeStepper/IterationAdaptiveDT/post_function_sync_dt)
input parameter as well.

!media media/executioner/adaptive_dt_criteria.png style=width:70%; id=fig:adaptive_dt_criteria caption=Criteria used to determine adaptive time step size

!alert tip
The `IterationAdaptiveDT` is often simply used to have an exponentially growing time step. For this
purpose, the iteration related parameters are not required.

## Example Input Syntax

!listing test/tests/time_steppers/iteration_adaptive/adapt_tstep_shrink_init_dt.i block=Executioner

!syntax parameters /Executioner/TimeStepper/IterationAdaptiveDT

!syntax inputs /Executioner/TimeStepper/IterationAdaptiveDT

!syntax children /Executioner/TimeStepper/IterationAdaptiveDT
