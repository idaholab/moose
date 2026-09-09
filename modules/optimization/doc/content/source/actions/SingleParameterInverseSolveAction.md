# SingleParameterInverseSolveAction

!syntax description /SingleParameterInverseSolve/SingleParameterInverseSolveAction

## Overview

The `SingleParameterInverseSolve` action generates a complete fixed-point inverse-solve workflow from a
single time-dependent scalar parameter, so users do not have to hand-write the coupled `MultiApps`, `Transfers`,
`Postprocessors`, `Convergence`, and `Controls` blocks. It finds the scalar parameter `p(t)` such
that a forward-model sub-application output matches a target [Function](Functions/index.md).

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
