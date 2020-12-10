[StochasticTools]
[]

[FormFunction]
  type = ObjectiveMinimize
  parameter_names = 'parameter_results'
  num_values = '4'
  initial_condition = '100 1 -10 -10'

  data_computed = 'measured/values'
  data_target = '209 218 164 121'
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
    reset_app = true
  []
[]

[Controls]
  [toforward]
    type = OptimizationMultiAppCommandLineControl
    multi_app = forward
    value_names = 'parameter_results'
    parameters = 'Functions/volumetric_heat_func/vals'
  []
[]

[Transfers]
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'measured/values'
  []
[]

[Reporters]
  [measured]
    type = ConstantReporter
    real_vector_names = values
    real_vector_values = '0 0 0 0'
  []
[]

[Outputs]
  console = true
  csv=true
[]
