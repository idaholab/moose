[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveMinimize
  parameter_names = 'parameter_results'
  num_values = '1'
  initial_condition = '1000'
  lower_bounds = '0.1'
  upper_bounds = '2000'
  measurement_points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measurement_values = '226 254 214 146'

[]

[Executioner]
  type = Optimize
  tao_solver = TAONM
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
[]

[Transfers]
  #this is usually the same for all input files.
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data_pt/temperature data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
  []
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  #This depends on parameter being optimized
  [toforward]
    type = OptimizationParameterTransfer
    to_multi_app = forward
    to_control = parameterReceiver
    value_names = 'parameter_results'
    parameters = 'Postprocessors/p1/value'
  []


[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0'
  []
[]

[Outputs]
  #console = true
  csv=true
[]
