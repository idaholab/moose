# SingleParameterInverseSolve System

The `SingleParameterInverseSolve` block generates a complete fixed-point (Picard) inverse-solve
workflow for a single time-dependent scalar parameter from one block.

See [SingleParameterInverseSolveAction.md] for the workflow, the complete list of generated objects,
and the one required `[Executioner]` line
(`multiapp_fixed_point_convergence = single_parameter_inverse_solve_convergence`).

!syntax parameters /SingleParameterInverseSolve

!syntax list /SingleParameterInverseSolve objects=True actions=False subsystems=False

!syntax list /SingleParameterInverseSolve objects=False actions=True subsystems=False
