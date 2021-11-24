[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveMinimize
  parameter_names = 'parameter_results'
  num_values = '1'
  initial_condition = '1000'
  lower_bounds = '0'
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
  #verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
[]

[Transfers]
  [toforward]
    type = OptimizationParameterTransfer
    multi_app = forward
    to_control = parameterReceiver
    value_names = 'parameter_results'
    parameters = 'Postprocessors/p1/value'
  []

  #this is usually the same for all input files.
    [fromForward]
      type = MultiAppReporterTransfer
      multi_app = forward
      direction = from_multiapp
      from_reporters = 'data_pt/temperature data_pt/temperature'
      to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
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
