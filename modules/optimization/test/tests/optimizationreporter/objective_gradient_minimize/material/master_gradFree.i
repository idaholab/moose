[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]

[OptimizationReporter]
  type = ObjectiveMinimize
  parameter_names = 'p1'
  num_values = '1'
  initial_condition = '8'
  lower_bounds = '0'
  upper_bounds = '10'
  measurement_points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measurement_values = '226 254 214 146'
[]

[Executioner]
  type = Optimize
  tao_solver = taonm
  petsc_options_iname = '-tao_max_it -tao_gatol'
  petsc_options_value = '100 1e-4'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_master_mesh = true
    ignore_solve_not_converge = true #false
  []
[]

[Transfers]
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

  [toforward]
    type = OptimizationParameterTransfer
    multi_app = forward
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []

[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0'
   []
   [optInfo]
     type = OptimizationInfo
     items = 'current_iterate'
   []
[]


[Outputs]
  file_base = 'master_gradFree'
  console = false
  csv=true
[]
