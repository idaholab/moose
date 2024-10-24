[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'D'
  num_values = '4'
  initial_condition = '0.2 0.2 0.2 0.2'
[]

[Reporters]
  [main]
    type = OptimizationData

    measurement_file = forward_out_data_0011.csv
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
    input_files = forward.i
    cli_args = 'Outputs/csv=false;Outputs/console=false'
    execute_on = FORWARD
  []
[]

[Transfers]
  [to_forward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'main/measurement_values
                      OptimizationReporter/D'
    to_reporters = 'data/measurement_values
                    diffc_rep/D_vals'
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data/objective_value'
    to_reporters = 'OptimizationReporter/objective_value'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taonm
  petsc_options_iname = '-tao_gatol -tao_nm_lambda'
  petsc_options_value = '1e-8 0.25'
  verbose = true
[]
