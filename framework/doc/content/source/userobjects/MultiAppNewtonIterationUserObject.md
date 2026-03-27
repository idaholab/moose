# MultiAppNewtonIterationUserObject

!syntax description /UserObjects/MultiAppNewtonIterationUserObject

## Description

`MultiAppNewtonIterationUserObject` solves an inverse problem across a single internally-managed
`TransientMultiApp` instance: given a target time-dependent output value $f_\text{target}(t)$,
it finds the time-dependent scalar parameter $p(t)$ such that the sub-app output $f_\text{calc}[p(t), t]$
equals the target $f_\text{target}(t)$ at each time step.

!alert note
Newton's method converges only when the output, $f_\text{calc}[p(t), t]$, depends on the parameter,
$p(t)$, in an at least $\bm{C^1}$ continuous manner. If the output is discontinuous or non-smooth in
$p(t)$, the finite-difference derivative may be meaningless and the iteration may fail to converge.

The sub-app runs the input file specified by
[!param](/UserObjects/MultiAppNewtonIterationUserObject/sub_app_input).
The user does not need a `MultiApps` block — the `TransientMultiApp` instance is created
automatically with `execute_on = NONE` so that only this UserObject controls its execution.

### Algorithm

At each time step the following procedure is performed:

1. +Backup+ the sub-app at the start of the time step ($t$) so every Newton trial starts from the
   same initial condition.
2. +Newton iteration+ using a finite-difference Jacobian. Each iteration performs two solves on
   the same sub-app, restoring the start-of-timestep state between them:

   - A perturbed solve with parameter $p_n(t) + \delta p$
     ([!param](/UserObjects/MultiAppNewtonIterationUserObject/delta_parameter)), giving $y_2=f_\text{calc}[p_n(t) + \delta p, t]$.
   - A base solve with parameter $p_n(t)$, giving $y_1=f_\text{calc}[p_n(t), t]$.
   - The derivative $\mathrm{d}f_\text{calc}/\mathrm{d}p \approx (y_2 - y_1)/\delta p$ is estimated.
   - The Newton update $p_{n+1}(t) = p_n(t) - (y_1 - f_\text{target}(t)) / (\mathrm{d}f_\text{calc}/\mathrm{d}p)$ is applied.
   - Iteration continues until $|y_1 - f_\text{target}(t)| < \max(\texttt{abs\_tol},\ \texttt{rel\_tol} \cdot |f_\text{target}(t)|)$,
     or [!param](/UserObjects/MultiAppNewtonIterationUserObject/max_iterations) is reached. If the
     iteration count is exceeded without convergence, the time step is cut and retried (the run
     errors once the time step can no longer be reduced) unless
     [!param](/UserObjects/MultiAppNewtonIterationUserObject/accept_on_max_iteration) is set to
     `true`, in which case the best estimate is accepted with a warning.

3. +Advance+ the sub-app one time step at the converged $p(t)$.

### Sequential vs. concurrent perturbation

By default the two evaluations ($p_n(t)$ and $p_n(t) + \delta p$) reuse a single sub-app, solved twice per
iteration with the start-of-timestep state restored in between. This uses the least memory. The
base ($p_n(t)$) solve is performed last, so on convergence the sub-app already holds the $p_n(t)$-solution
and only an extra solve at the reported $p_n(t)$ is needed if `max_iterations` is reached.

Setting [!param](/UserObjects/MultiAppNewtonIterationUserObject/concurrent_perturbation) to `true`
instead creates the MultiApp with two sub-apps (one for $p_n(t)$, one for $p_n(t) + \delta p$). MOOSE
distributes these across the available MPI ranks, so both sub-app solves can be performed
simultaneously, roughly halving the per-iteration wall-clock time. This requires `>= 2` MPI
processes to be faster and uses roughly twice the sub-app memory; on a single process the two
sub-apps still solve sequentially. The numerical result is identical to the sequential mode.

The converged parameter value is stored as restartable data and is used as the initial guess for
the next time step. It can also be written to an optional `Receiver` postprocessor in the main app
via [!param](/UserObjects/MultiAppNewtonIterationUserObject/parameter_postprocessor).

### Communication

The parameter is communicated to the sub-app by directly setting a `Receiver` postprocessor
(named by [!param](/UserObjects/MultiAppNewtonIterationUserObject/param_postprocessor)) in the sub-app. The output is read from a postprocessor named by
[!param](/UserObjects/MultiAppNewtonIterationUserObject/output_postprocessor).

## Example input syntax

In this example, the `MultiAppNewtonIterationUserObject` finds the parameter value each time step
such that the sub-app ODE solution matches a target function $f_\text{target}(t) = t^2$.

!listing test/tests/userobjects/multiapp_newton_iteration/main.i block=UserObjects

!syntax parameters /UserObjects/MultiAppNewtonIterationUserObject

!syntax inputs /UserObjects/MultiAppNewtonIterationUserObject

!syntax children /UserObjects/MultiAppNewtonIterationUserObject
