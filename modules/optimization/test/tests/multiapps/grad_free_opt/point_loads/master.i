[StochasticTools]
[]

[FormFunction]
  type = ObjectiveMinimize
  parameter_names = 'parameter_results'
  num_values = '3'
  initial_condition = '-2450 7250 26330' # close to answer

  data_computed_name = 'measured'
  data_target = '100 204 320 216'
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
    execute_on = forward
  []
[]

[Transfers]
  [toforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = to_multiapp
    from_reporters = 'FormFunction/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'FormFunction/measured'
  []
[]

[Outputs]
  console = true
  csv=true
[]
