# Hand-written driver for NewtonInversionControl.
#
# Defaults match secant.i (linear forward model, target f(t) = t^2, initial guess p0 = 1) so the
# two Controls can be compared on the same problem. Forward map (per step): y(p) = u_prev + p, so
# the finite-difference derivative is exact and Newton reaches the root in one step. Expected
# param_value = 1, 3, 5, 7, 9 at t = 1..5.
#
# The newton_nonlinear test reuses this input with cli_args to swap in the cubic forward model,
# which secant cannot solve (see the secant_fails_on_nonlinear test).
#
# NewtonInversionControl uses two fixed-point iterations per Newton step (base solve at p,
# perturbed solve at p + parameter_delta), so max_iterations must allow for both.

# Supplies the Mesh and a Problem that skips the nonlinear-system check, so this orchestrating
# main app needs no variables of its own.
[Optimization]
[]

[Functions]
  [target_fn]
    type = ParsedFunction
    expression = 't^2'
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = forward_linear.i
    execute_on = TIMESTEP_BEGIN
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = p
    to_postprocessor = param_pp
  []
  [from_sub]
    type = MultiAppPostprocessorTransfer
    from_multi_app = sub
    from_postprocessor = output_pp
    to_postprocessor = output
    reduction_type = average
  []
[]

[Postprocessors]
  # Working parameter guess: transferred to the sub, read and updated by the Control.
  [p]
    type = Receiver
    default = 1.0
    outputs = none
  []
  # Sub-app output, filled by the FROM transfer.
  [output]
    type = Receiver
    outputs = none
  []
  # Convergence residual written by the Control (|y_base - target| on base iterations,
  # a large sentinel on perturbed iterations).
  [residual]
    type = Receiver
    default = 1e30
    outputs = none
  []
  # Published converged parameter (the inverse-problem solution). This is the CSV output.
  [param_value]
    type = Receiver
    default = 0.0
  []
[]

[Controls]
  [newton]
    type = NewtonInversionControl
    output_postprocessor = output
    parameter_postprocessor = p
    residual_postprocessor = residual
    converged_parameter_postprocessor = param_value
    target_function = target_fn
    parameter_delta = 1e-3
  []
[]

[Convergence]
  [inv_conv]
    type = PostprocessorConvergence
    postprocessor = residual
    tolerance = 1.0
    max_iterations = 50
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  solve_type = NEWTON
  multiapp_fixed_point_convergence = inv_conv
[]

[Outputs]
  csv = true
[]
