[Optimization]
[]

[OptimizationReporter]
  type = ParameterMeshOptimization
  parameter_names = 'reaction_rate'
  parameter_meshes = 'parameter_mesh_out.e'

  constant_group_initial_condition = 0
  constant_group_lower_bounds = 0

  measurement_file = forward_exact_csv_sample_0011.csv
  file_xcoord = measurement_xcoord
  file_ycoord = measurement_ycoord
  file_zcoord = measurement_zcoord
  file_time = measurement_time
  file_value = simulation_values
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
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
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
    from_reporters = 'data/simulation_values
                      adjoint/inner_product'
    to_reporters = 'OptimizationReporter/simulation_values
                    OptimizationReporter/grad_reaction_rate'
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
  #petsc_options_value = '1e-5 100' #use this to get results for paper
  petsc_options_value = '1e-5 5'
  solve_on = 'NONE'
  verbose = true
[]
[Outputs]
  csv = true
[]
