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
[AuxVariables]
  [T_forward]
  []
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '1'
  initial_condition = '500'
  lower_bounds = '0.1'
  upper_bounds = '10000'
  measurement_points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measurement_values = '270 339 321 221'
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '.01'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward_nonLinear.i
    execute_on = FORWARD
    reset_app = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint_nonLinear.i
    execute_on = ADJOINT
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
  [toForward_measument]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = from_multiapp
    from_reporters = 'data_pt/T'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'gradient_vpp/gradient_vpp'
    to_reporters = 'OptimizationReporter/adjoint'
  []
  #for temperature dependent material
  [fromforwardMesh]
    type = MultiAppCopyTransfer
    multi_app = forward
    direction = from_multiapp
    source_variable = 'T'
    variable = 'T_forward'
  []
  [toAdjointMesh]
    type = MultiAppCopyTransfer
    multi_app = adjoint
    direction = to_multiapp
    source_variable = 'T_forward'
    variable = 'T'
  []
[]

[Reporters]
   [optInfo]
     type = OptimizationInfo
   []
[]

[Outputs]
  # console = true
  csv=true
  # exodus = true
[]
