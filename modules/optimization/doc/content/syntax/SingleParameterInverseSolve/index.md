# SingleParameterInverseSolve System

The `SingleParameterInverseSolve` block generates a complete fixed-point (Picard) inverse-solve
workflow for a single time-dependent scalar parameter, so users do not have to hand-write the
coupled `[MultiApps]`, `[Transfers]`, `[Postprocessors]`, `[Convergence]`, and `[Controls]` blocks.
It finds the scalar parameter `p(t)` such that a forward-model sub-application output matches a
target [Function](Functions/index.md), using a secant or finite-difference Newton update selected by
the `method` parameter.

See [SingleParameterInverseSolveAction.md] for the complete list of generated objects and the one
required `[Executioner]` line
(`multiapp_fixed_point_convergence = single_parameter_inverse_solve_convergence`).

!listing test/tests/controls/inverse_solve_action/action_secant.i block=SingleParameterInverseSolve

!syntax parameters /SingleParameterInverseSolve

!syntax list /SingleParameterInverseSolve objects=True actions=False subsystems=False

!syntax list /SingleParameterInverseSolve objects=False actions=True subsystems=False
