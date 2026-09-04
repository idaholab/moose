# Driver for SecantInversionControl.
#
# Finds param p(t) such that the sub-app output u(t) matches target f(t) = t^2.
# Analytic solution (dt = 1): p(t) = 2*t - 1  =>  u = 1, 4, 9, 16, 25 at t = 1..5.
# The converged parameter is published to `param_value` (1, 3, 5, 7, 9).

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  # Placeholder so the main-app nonlinear system is non-empty.
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

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
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
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  # Sub-app output, filled by the FROM transfer.
  [output]
    type = Receiver
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  # Convergence residual, written (normalized) by the Control each iteration.
  [residual]
    type = Receiver
    default = 1e30
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  # Published converged parameter (the inverse-problem solution). This is the CSV output.
  [param_value]
    type = Receiver
    default = 0.0
  []
[]

[Controls]
  [secant]
    type = SecantInversionControl
    output_postprocessor = output
    parameter_postprocessor = p
    residual_postprocessor = residual
    converged_parameter_postprocessor = param_value
    target_function = target_fn
    initial_delta = 1e-3
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
