[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'D'
  num_values = '4'
  initial_condition = '1 1 1 1'
  upper_bounds = '1e3'
  lower_bounds = '1e-2'

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
  [adjoint]
    type = FullSolveMultiApp
    input_files = gradient.i
    cli_args = 'Outputs/console=false;UserObjects/load_u/mesh=optimize_grad_out_forward0.e'
    execute_on = ADJOINT
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
                      OptimizationReporter/D'
    to_reporters = 'data/measurement_xcoord
                    data/measurement_ycoord
                    data/measurement_zcoord
                    data/measurement_time
                    data/measurement_values
                    diffc_rep/D_vals'
  []
  [from_forward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  # Need this so that the final forward solution is set in the initial time step of the adjoint simulation
  [forward_to_adjoint]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    to_multi_app = adjoint
    source_variable = u
    variable = u
  []
  [to_adjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/misfit_values
                      OptimizationReporter/D'
    to_reporters = 'data/measurement_xcoord
                    data/measurement_ycoord
                    data/measurement_zcoord
                    data/measurement_time
                    data/misfit_values
                    diffc_rep/D_vals'
  []
  [from_adjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint/inner_product'
    to_reporters = 'OptimizationReporter/grad_D'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-4'
  verbose = true
[]
