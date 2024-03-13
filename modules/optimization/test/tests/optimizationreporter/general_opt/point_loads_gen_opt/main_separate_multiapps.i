measurement_points = '0.5 0.28 0
   0.5 0.6 0
   0.5 0.8 0
   0.5 1.1 0'
measurement_values = '293 304 315 320'

[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = misfit_norm
  parameter_names = 'parameter_results'
  num_values = '3'
[]
[Reporters]
  [main]
    # We need to have an OptimizationData on the main app to allow the
    # transferring of the correct information when doing Hessian based optimization.
    type = OptimizationData
    measurement_points = ${measurement_points}
    measurement_values = ${measurement_values}
  []
[]
[Executioner]
  type = Optimize
  tao_solver = taonls
  petsc_options_iname = '-tao_gttol -tao_max_it -tao_nls_pc_type -tao_nls_ksp_type'
  petsc_options_value = '1e-5 10 none cg'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    cli_args = 'measurement_points="${measurement_points}";measurement_values="${measurement_values}"'
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    cli_args = 'measurement_points="${measurement_points}";measurement_values="${measurement_values}"'
  []
  [homogeneousForward]
    type = FullSolveMultiApp
    input_files = forward_homogeneous.i
    execute_on = "HOMOGENEOUS_FORWARD"
    cli_args = 'measurement_points="${measurement_points}";measurement_values="${measurement_values}"'
  []
[]

[Transfers]
  # FORWARD transfers
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    # Note: We are transferring the misfit values into main misfit
    from_reporters = 'measure_data/misfit_norm measure_data/misfit_values'
    to_reporters = 'OptimizationReporter/misfit_norm main/misfit_values'
  []

  # ADJOINT transfers
  #NOTE:  the adjoint variable we are transferring is actually the gradient
  [toAdjoint]
    type = MultiAppReporterTransfer
    # We are transferring directly from the forward app to the adjoint app
    to_multi_app = adjoint
    from_reporters = 'main/misfit_values'
    to_reporters = 'misfit/misfit_values'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradient/adjoint'
    to_reporters = 'OptimizationReporter/grad_parameter_results'
  []

  # HESSIAN transfers.  Same as forward.
  [toHomoForward]
    type = MultiAppReporterTransfer
    to_multi_app = homogeneousForward
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromHomoForward]
    type = MultiAppReporterTransfer
    from_multi_app = homogeneousForward
    # Note: We are transferring the simulation values into misfit
    # this has to be done when using general opt and homogenous forward.
    from_reporters = 'measure_data/simulation_values'
    to_reporters = 'main/misfit_values'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
  []
[]

[Outputs]
  csv = true
  file_base = main_out
[]
