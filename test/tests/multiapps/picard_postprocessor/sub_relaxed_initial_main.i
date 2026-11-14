# Tight coupling of two scalar equations, relaxed on the sub-application side.
#   main: x = 1 + y/2     sub: y = 1 + x/4
# The fixed point is x = 12/7, y = 10/7.
#
# The sub-application relaxes the postprocessor it receives. The relaxation at the first
# iteration must use the initial value of that postprocessor, not zero, so the number of
# iterations is pinned to keep the relaxed values away from the converged solution.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [x]
    family = SCALAR
    order = FIRST
    initial_condition = 1.0
  []
[]

[ScalarKernels]
  [ode]
    type = ParsedODEKernel
    variable = x
    expression = 'x - 1 - 0.5 * y_recv'
    postprocessors = 'y_recv'
  []
[]

[Postprocessors]
  [y_recv]
    type = Receiver
    default = 1.0
    execute_on = 'initial timestep_begin'
  []
  [x_out]
    type = ScalarVariable
    variable = x
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-12

  fixed_point_max_its = 20
  # loose, so the solve terminates while the relaxed values are still away from the
  # fixed point and therefore still sensitive to the first relaxed update
  fixed_point_rel_tol = 1e-2
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = sub_relaxed_initial_sub.i
    execute_on = 'timestep_begin'
    # we keep the full postprocessor output history of the subapp
    keep_full_output_history = true

    relaxation_factor = 0.8
    transformed_postprocessors = 'x_recv'
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppPostprocessorTransfer
    from_multi_app = sub
    from_postprocessor = 'y_out'
    to_postprocessor = 'y_recv'
    reduction_type = 'average'
  []
  [to_sub]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = 'x_out'
    to_postprocessor = 'x_recv'
  []
[]

[Outputs]
  csv = true
[]
