[Optimization]
[]

[OptimizationReporter]
  type = ParameterMeshOptimization
  objective_name = objective_value
  parameter_names = 'reaction_rate'
  parameter_meshes = 'parameter_mesh_out.e'
  lower_bounds = 0
[]

[Reporters]
  [main]
    type = OptimizationData
    measurement_file = forward_exact_csv_sample_0011.csv
    file_xcoord = measurement_xcoord
    file_ycoord = measurement_ycoord
    file_zcoord = measurement_zcoord
    file_time = measurement_time
    file_value = simulation_values
  []
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [to_forward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/measurement_values
                      OptimizationReporter/reaction_rate'
    to_reporters = 'data/measurement_xcoord
                    data/measurement_ycoord
                    data/measurement_zcoord
                    data/measurement_time
                    data/measurement_values
                    params/reaction_rate'
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'adjoint/inner_product data/objective_value'
    to_reporters = 'OptimizationReporter/grad_reaction_rate OptimizationReporter/objective_value'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
    items = 'current_iterate function_value gnorm'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  petsc_options_iname = '-tao_gttol -tao_max_it'
  petsc_options_value = '1e-5 5'

  # petsc_options_iname='-tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta'
  # petsc_options_value='true true false 1e-8'
  # petsc_options = '-tao_test_gradient_view'
  solve_on = 'NONE'
  verbose = true
[]
[Outputs]
  csv = true
[]
