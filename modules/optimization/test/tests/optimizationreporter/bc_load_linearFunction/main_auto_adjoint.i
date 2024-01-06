# This tests that a linear and constant function can be scaled in
# two separate functionNeumannBCs both applied to the same sideset using
# two parsed functions.  The scale of the linear and constant functions
# are being parameterized.

[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  parameter_names = 'left right'
  num_values = '2 1'
  objective_name = obj_value
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  petsc_options_iname = '-tao_gatol -tao_ls_type'
  petsc_options_value = '1e-5 unit'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = "FORWARD"
  []
[]

[Transfers]
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/left
                      OptimizationReporter/right'
    to_reporters = 'params/left
                    params/right'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'obj_sum/value
                      grad_bc_left/inner_product
                      grad_bc_right/inner_product'
    to_reporters = 'OptimizationReporter/obj_value
                    OptimizationReporter/grad_left
                    OptimizationReporter/grad_right'
  []
[]
