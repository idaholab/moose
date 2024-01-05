[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = misfit_norm
  parameter_names = 'parameter_results'
  num_values = '3'
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
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
  []
  [homogeneousForward]
    type = FullSolveMultiApp
    input_files = forward_homogeneous.i
    execute_on = "HOMOGENEOUS_FORWARD"
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
    multi_app = homogeneousForward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromHomoForward]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = from_multiapp
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
  [main]
    # We need to have an OptimizationData on the main app to allow the
    # transferring of the correct information.
    type = OptimizationData
    !include measure_data.i
  []
[]

[Outputs]
  csv = true
[]
