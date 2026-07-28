# Parity driver for NewtonInversionControl on the linear forward model.
#
# Forward map (per step): y(p) = u_prev + p.  Target f(t) = t^2  =>  p(t) = 2*t - 1.
# The finite-difference derivative is exact for a linear map, so Newton reaches the
# root in one step. Expected param_value = 1, 3, 5, 7, 9 at t = 1..5 (matches the
# SecantInversionControl inverse_solve gold).

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

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub_linear.i
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
  [p]
    type = Receiver
    default = 1.0
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  [output]
    type = Receiver
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
  [residual]
    type = Receiver
    default = 1e30
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    outputs = none
  []
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
