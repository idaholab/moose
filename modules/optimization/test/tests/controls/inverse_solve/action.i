# Inverse solve driven by the `SingleParameterInverseSolve` action.
#
# Equivalent to the hand-written secant.i workflow: finds p(t) so the forward output u(t) matches
# target f(t) = t^2. Expected param_value = 1, 3, 5, 7, 9.
#
# The block generates the forward MultiApp, the transfers, the working postprocessors, the
# convergence, and the inversion Control. The only remaining wiring is the one required executioner
# line pointing at the generated convergence.
#
# The tests reuse this input with cli_args to reach the Newton method, the accept-on-max and
# absolute-tolerance paths, and the error raised when the executioner line is wrong.

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

[SingleParameterInverseSolve]
  method = secant
  forward_input = forward_linear.i
  sub_parameter_postprocessor = param_pp
  sub_output_postprocessor = output_pp
  target_function = target_fn
  initial_parameter = 1.0
  result_postprocessor = param_value
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  solve_type = NEWTON
  multiapp_fixed_point_convergence = single_parameter_inverse_solve_convergence
[]

[Outputs]
  csv = true
[]
