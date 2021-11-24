[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '3'
  measurement_points = '0.0 0.3 0
            0.0 0.5 0
            0.0 1.0 0'
  measurement_values = '100 200 300'
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gatol -tao_ls_type'
  petsc_options_value = '1e-4 unit'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
  []
[]

[Transfers]
  #these are usually the same for all input files.
    [fromForward]
      type = MultiAppReporterTransfer
      multi_app = forward
      direction = from_multiapp
      from_reporters = 'data_pt/temperature data_pt/temperature'
      to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
    []
    [toAdjoint]
      type = MultiAppReporterTransfer
      multi_app = adjoint
      direction = to_multiapp
      from_reporters = 'OptimizationReporter/measurement_points OptimizationReporter/misfit_values'
      to_reporters = 'misfit/measurement_points misfit/misfit_values'
    []


  [toforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0 0 0'
  []
[]

[Outputs]
  console = true
  csv=true
[]
