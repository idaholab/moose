[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
  bias_x = 1.0
  bias_y = 1.0
[]

[FormFunction]
  type = ObjectiveMinimize
  parameter_names = 'bc_left bc_right'
  num_values = '1 1'
  misfit_name = 'misfit'
[]

[Executioner]
  type = Optimize
  tao_solver = TAONM
[]


[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_master_mesh = true
  []
[]

[Transfers]
  [toforward]
    type = OptimizationParameterTransfer
    multi_app = forward
    value_names = 'bc_left bc_right'
    parameters = 'BCs/left/value BCs/right/value'
    to_control = parameterReceiver
  []
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    from_reporters = 'data_pt/temperature_difference data_pt/temperature'
    to_reporters = 'FormFunction/misfit receiver/measured'
    direction = from_multiapp
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0 0 0 0'
  []
[]

[Outputs]
  csv = true
[]
