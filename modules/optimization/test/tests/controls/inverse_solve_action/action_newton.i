# Newton inverse solve driven by the `SingleParameterInverseSolve` action.
#
# Nonlinear forward model du/dt = p^3, target f(t) = 1000*t (root p = 10 each step),
# initial guess p0 = -3. Equivalent to controls/newton_inverse_solve/newton_nonlinear.
# Expected param_value = 10 at every step.

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
    expression = '1000*t'
  []
[]

[SingleParameterInverseSolve]
  method = newton
  forward_input = forward_cubic.i
  sub_parameter_postprocessor = param_pp
  sub_output_postprocessor = output_pp
  target_function = target_fn
  initial_parameter = -3.0
  perturbation = 1e-3
  max_iterations = 25
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
