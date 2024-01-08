[Optimization]
[]

[OptimizationReporter]
  type = ParameterMeshOptimization
  objective_name = objective_value
  parameter_names = 'source'
  parameter_meshes = source_mesh_in.e
  num_parameter_times = 11
[]

[Reporters]
  [main]
    type = OptimizationData
  []
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_mesh.i
    execute_on = FORWARD
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint_mesh.i
    execute_on = ADJOINT
  []
[]

[Transfers]
  [to_forward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/source'
    to_reporters = 'src_values/values'
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measured_data/misfit_values measured_data/objective_value'
    to_reporters = 'main/misfit_values OptimizationReporter/objective_value'
  []
  [to_adjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/source main/misfit_values'
    to_reporters = 'src_values/values measured_data/misfit_values'
  []
  [from_adjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint/inner_product'
    to_reporters = 'OptimizationReporter/grad_source'
  []
[]

[Executioner]
  type = Optimize
  solve_on = none
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gatol -tao_ls_type'
  petsc_options_value = '1e-2 unit'
  verbose = true
[]
