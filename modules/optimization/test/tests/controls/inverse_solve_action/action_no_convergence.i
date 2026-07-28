# Same as action_secant.i but WITHOUT the required
# 'multiapp_fixed_point_convergence = single_parameter_inverse_solve_convergence' line in the
# [Executioner]. The action should catch this misconfiguration and error with guidance rather
# than silently running with no fixed-point loop.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [dummy]
    family = SCALAR
    order = FIRST
    initial_condition = 0.0
  []
[]

[ScalarKernels]
  [null]
    type = NullScalarKernel
    variable = dummy
  []
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
