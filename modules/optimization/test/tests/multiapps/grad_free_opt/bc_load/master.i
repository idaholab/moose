[StochasticTools]
[]

[FormFunction]
  type = ObjectiveMinimize
  parameter_names = 'bc_left bc_right'
  num_values = '1 1'

  data_computed = 'measured/values'
  data_target = '199 214 154 129'
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
  [toforward]
    type = OptimizationParameterTransfer
    multi_app = forward
    to_control = parameterReceiver

    value_names = 'bc_left bc_right'
    parameters = 'BCs/left/value BCs/right/value'
  []
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
