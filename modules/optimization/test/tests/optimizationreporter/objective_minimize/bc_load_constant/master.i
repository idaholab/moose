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

[OptimizationReporter]
  type = ObjectiveMinimize
  parameter_names = 'bc_left bc_right'
  num_values = '1 1'
  measurement_points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measurement_values = '199 214 154 129'
[]

[Executioner]
  type = Optimize
  tao_solver = TAONM
  petsc_options_iname = '-tao_gatol'#
  petsc_options_value = '1e-4'
  verbose = true
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
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [fromforward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
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
  csv = true
[]
