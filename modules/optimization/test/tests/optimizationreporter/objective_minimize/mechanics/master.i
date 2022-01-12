[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
  xmin = 0.0
  xmax = 5.0
  ymin = 0.0
  ymax = 1.0
[]

[OptimizationReporter]
  type = ObjectiveMinimize
  parameter_names = 'fy_right'
  num_values = '1'
  measurement_points = '5.0 1.0 0.0'
  measurement_values = '0.000080935883542921'
  initial_condition = '100'
[]

[Executioner]
  type = Optimize
  tao_solver = TAONM
  petsc_options_iname = '-tao_gatol'#
  petsc_options_value = '1e-16'
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
    value_names = 'fy_right'
    parameters = 'BCs/right_fy/value'
    to_control = parameterReceiver
  []
  [toForward_measument]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    from_reporters = 'data_pt/disp_y data_pt/disp_y'
    to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
    direction = from_multiapp
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
