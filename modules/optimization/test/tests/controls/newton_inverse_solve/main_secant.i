# Companion to main_newton.i: the SAME nonlinear inverse problem (y(p) = p^3, target
# 1000, initial guess p0 = -3) solved with SecantInversionControl instead of Newton.
#
# The secant method never converges here: after its first (locally seeded) step it
# estimates df/dp from two far-apart iterates that straddle the inflection at p = 0,
# so it enters a bounded chaotic orbit (residual stuck ~1000). With dtmin = dt the
# step cannot be cut, so the run errors once max_iterations is exhausted. This
# demonstrates a problem the finite-difference Newton control solves but secant cannot.

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
    expression = '1000'
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
  [p]
    type = Receiver
    default = -3.0
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
    max_iterations = 25
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  dtmin = 1
  solve_type = NEWTON
  multiapp_fixed_point_convergence = inv_conv
[]

[Outputs]
  csv = true
[]
