# Same as action.i but WITHOUT the required 'multiapp_fixed_point_convergence' line in the
# [Executioner]. Since this omission is what makes MOOSE silently create and assign a default
# fixed-point convergence, this must be a genuine omission -- overriding the line via cli_args (as
# action_wrong_convergence.i does with a real but wrong name) cannot exercise this path, only
# replace an existing value.
#
# The action should catch this misconfiguration and error with guidance rather than silently
# running with no fixed-point loop.

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
[]

[Outputs]
  csv = true
[]
