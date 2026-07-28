# Secant inverse solve driven by the `SingleParameterInverseSolve` action.
#
# Equivalent to the hand-written controls/inverse_solve workflow: finds p(t) so the
# forward output u(t) matches target f(t) = t^2. Expected param_value = 1,3,5,7,9.
#
# The `SingleParameterInverseSolve` block generates the forward MultiApp, the transfers, the working
# postprocessors, the convergence, and the SecantInversionControl. The only remaining
# wiring is the one required executioner line pointing at the generated convergence.

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
  multiapp_fixed_point_convergence = single_parameter_inverse_solve_convergence
[]

[Outputs]
  csv = true
[]
