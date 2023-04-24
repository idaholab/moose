[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'D'
  num_values = '4'
  initial_condition = '0.01 0.01 0.01 0.01'

  measurement_file = forward_out_data_0011.csv
  file_xcoord = measurement_xcoord
  file_ycoord = measurement_ycoord
  file_zcoord = measurement_zcoord
  file_time = measurement_time
  file_value = simulation_values
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
    from_reporters = 'OptimizationReporter/measurement_values
                      OptimizationReporter/D'
    to_reporters = 'data/measurement_values
                    diffc_rep/D_vals'
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taonm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-6'
  verbose = true
[]
