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
  [temperature_forward]
  []
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
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
  # FINITE DIFFERENCE OPTIONS
  # petsc_options_iname='-tao_max_it -tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value='1000 true 0.00001 0.0001'
  #-------------
  # petsc_options_iname = '-tao_gatol'
  # petsc_options_value = '1e-4'
  # petsc_options_iname = '-tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value = 'true 0.0001 1e-4'
   # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
   # petsc_options_value='1            true         true               false            0.00001       0.0001'
   petsc_options_iname='-tao_gatol'
   petsc_options_value='0.0001'
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
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_master_mesh = true
    ignore_solve_not_converge = false
  []
[]

[Transfers]
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data_pt/temperature data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
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
  [fromforwardMesh]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    source_variable = 'temperature'
    variable = 'temperature_forward'
  []

  [toAdjointMesh]
    type = MultiAppCopyTransfer
    to_multi_app = adjoint
    source_variable = 'temperature_forward'
    variable = 'temperature_forward'
  []
  [toAdjointParameter]
    type = OptimizationParameterTransfer
    multi_app = adjoint
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_grad/adjoint_grad'
    to_reporters = 'OptimizationReporter/adjoint'
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
  file_base = 'master'
  console = true
  csv=true
[]
