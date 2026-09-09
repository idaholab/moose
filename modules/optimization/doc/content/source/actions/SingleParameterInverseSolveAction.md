# SingleParameterInverseSolveAction

!syntax description /SingleParameterInverseSolve/SingleParameterInverseSolveAction

## Overview

The `SingleParameterInverseSolve` action generates a complete fixed-point inverse-solve workflow
from a single block, so users do not have to hand-write the coupled `MultiApps`, `Transfers`,
`Postprocessors`, `Convergence`, and `Controls` blocks.

It recovers a single time-dependent scalar parameter $p(t)$ that drives a transient forward model to
match a target [Function](Functions/index.md) of time. The parameter is genuinely time-dependent,
but it can only be recovered at the time steps actually taken: at each step the workflow performs one
independent scalar root find for the value $p(t_n)$ that makes the forward output match the target at
that step, starting from the previous step's converged value. The recovered history is therefore the
sequence of samples $p(t_1), p(t_2), \ldots$, and it depends on the time discretization -- because
the forward model carries state from one step to the next, refining `dt` changes the recovered
values.

The forward model runs as a transient sub-application (a `TransientMultiApp`): the matched quantity
evolves in time and each step builds on the previous one, so the model must advance step by step and
be re-solved for each trial parameter inside the fixed-point (Picard) iteration. The main
application drives that loop -- it holds the parameter, transfers it down, reads the output back, and
updates the parameter through the inversion `Control`. It may also run its own physics, but need
not; the minimal setup only requires a non-empty nonlinear system on the main application.

This is deliberately not framed as an [Optimize](Optimize.md) solve. `Optimize` derives from
`Steady`: one objective evaluation runs the entire transient, and the full parameter history is
recovered simultaneously as a single optimization problem. `SingleParameterInverseSolve` instead
performs one independent one-dimensional root find per time step, nested in the executioner's
existing fixed-point (Picard) loop, and needs no additional application instances. The step-by-step
approach is applicable whenever the parameter can be recovered one step at a time, which holds when
the forward model advances step by step and each step depends only on the parameter at that step.

The `method` parameter selects the update rule:

- `secant` (default) &mdash; solve through [SecantInversionControl](SecantInversionControl.md).
- `newton` &mdash; solve through [NewtonInversionControl](NewtonInversionControl.md).

!alert note title=One per input
`SingleParameterInverseSolve` is a singular block: at most one may appear per input file, like
`Executioner` or `Mesh`. It drives the executioner's single
`multiapp_fixed_point_convergence`, so a single input runs exactly one single-parameter inverse
solve. To invert for a full parameter vector, use the module's
[OptimizationReporter](syntax/OptimizationReporter/index.md) / [Optimize](Optimize.md) machinery
instead.

### Action Behavior

The generated objects are named with a snake_case prefix derived from the block name (the
`SingleParameterInverseSolve` block yields the prefix `single_parameter_inverse_solve`). The
action creates:

- a `TransientMultiApp` (`single_parameter_inverse_solve_forward`) running the
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/forward_input) file;
- two [MultiAppPostprocessorTransfer](MultiAppPostprocessorTransfer.md)s passing the parameter down
  (to the sub-app's
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/sub_parameter_postprocessor))
  and the output back (from the sub-app's
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/sub_output_postprocessor));
- the working `Receiver` postprocessors (`single_parameter_inverse_solve_param`, seeded from
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/initial_parameter), and
  `single_parameter_inverse_solve_output`) and a result `Receiver` (named by
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/result_postprocessor),
  default `inverse_parameter`, output to CSV);
- a [PostprocessorConvergence](PostprocessorConvergence.md)
  (`single_parameter_inverse_solve_convergence`) driven by
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/max_iterations) and
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/accept_on_max_iterations);
  and the selected inversion [Control](syntax/Controls/index.md) (using
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/target_function) and
  [!param](/SingleParameterInverseSolve/SingleParameterInverseSolveAction/perturbation)).

!alert note title=Generated objects are not user-configurable
The action exposes only the parameters listed on this page. The generated `TransientMultiApp`,
`Transfers`, `Postprocessors`, `Convergence`, and `Control` are created with fixed settings, so
`TransientMultiApp` options such as `sub_cycling`, `max_procs_per_app`, `cli_args`, `catch_up`, and
`keep_solution_during_restore` cannot be reached through this block. A workflow that needs them
should be written out as explicit blocks rather than generated here.

### Required Executioner Parameter

Because the fixed-point loop is enabled when the executioner is constructed (before actions run),
one line must remain in the `[Executioner]` block to enable the loop and point it at the generated
convergence:

```
multiapp_fixed_point_convergence = single_parameter_inverse_solve_convergence
```

If this line is missing (or points at a different convergence), the action errors during setup with
a message telling users exactly how to fix the issue.

### Max Iterations Behavior

By default, if the fixed-point loop reaches `max_iterations` without converging, the solve diverges,
the executioner cuts the time step, and (if it cannot) errors. Set `accept_on_max_iterations = true`
to instead accept the current best estimate at `max_iterations` and continue; this forwards to the
generated convergence's `converge_at_max_iterations`.

## Example Input Syntax

!listing test/tests/controls/inverse_solve_action/action_secant.i

!syntax parameters /SingleParameterInverseSolve/SingleParameterInverseSolveAction
