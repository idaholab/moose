[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  parameter_names = 'radii'
  num_values_name = num_radii
  equality_names = 'volume_constraint'
  initial_condition = '0 0'
  lower_bounds = '-10'
  upper_bounds = '10'
  objective_name = max_temp
[]

[Executioner]
  type = Optimize
  tao_solver = taoalmm
  petsc_options_iname = '-tao_almm_subsolver_tao_type -tao_gatol -tao_catol  -tao_almm_type -tao_almm_mu_init -tao_fd_gradient -tao_fd_delta'
  petsc_options_value = 'bqnktr 1e-3 1e-3  phr 1e9 true 1e-8 '
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    # Run on initial so the forward problem can determine number of parameters
    execute_on = 'INITIAL FORWARD'
  []
[]

[Transfers]
  [to_forward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/radii'
    to_reporters = 'params/radii'
    execute_on = FORWARD
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'params/num_params
                      objective/value
                      vol_constraint/vol_constraint
                      eq_grad/eq_grad'
    to_reporters = 'OptimizationReporter/num_radii
                    OptimizationReporter/max_temp
                    OptimizationReporter/volume_constraint
                    OptimizationReporter/grad_volume_constraint'
  []
[]

[Outputs]
  csv = true
  print_linear_residuals = false
  print_nonlinear_residuals = false
[]
