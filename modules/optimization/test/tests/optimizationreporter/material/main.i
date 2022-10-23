[Optimization]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]
[AuxVariables]
  [temperature_forward]
  []
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'p1'
  num_values = '1'
  initial_condition = '7'
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
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '0.0001'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_parent_mesh = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_parent_mesh = true
  []
[]

[Transfers]
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [toForward]
    type = OptimizationParameterTransfer
    to_multi_app = forward
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []
  [fromForward_mesh]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    source_variable = 'temperature'
    variable = 'temperature_forward'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values'
  []

  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
  []
  [toAdjoint_mesh]
    type = MultiAppCopyTransfer
    to_multi_app = adjoint
    source_variable = 'temperature_forward'
    variable = 'temperature_forward'
  []
  [toAdjoint_parameter]
    type = OptimizationParameterTransfer
    to_multi_app = adjoint
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_grad/adjoint_grad'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Outputs]
  csv = true
[]
