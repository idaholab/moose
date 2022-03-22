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
  type = ObjectiveGradientMinimize
  parameter_names = 'fy_right'
  num_values = '1'
  measurement_points = '5.0 1.0 0.0'
  measurement_values = '80.9'
  initial_condition = '100'
[]

[Executioner]
  type = Optimize
  tao_solver = taonls
  petsc_options_iname = '-tao_gttol -tao_max_it -tao_nls_pc_type -tao_nls_ksp_type'
  petsc_options_value = '1e-5 50 none cg'
  verbose = true

#  type = Optimize
#  tao_solver = taolmvm
#  petsc_options_iname = '-tao_gatol -tao_grtol'
#  petsc_options_value = '1e-6 1e-6'
#  verbose = true

#  petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
#  petsc_options_value='1 true true false 1e-3 0.1'
#  petsc_options = '-tao_test_gradient_view'
[]


[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_master_mesh = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_master_mesh = true
  []
  # the forward problem has homogenous boundary conditions so it can be reused here.
  [homogenousForward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "HOMOGENOUS_FORWARD"
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
    from_reporters = 'data_pt/disp_y'
    to_reporters = 'OptimizationReporter/simulation_values'
    direction = from_multiapp
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    from_reporters = 'adjoint_pt/adjoint_pt'
    to_reporters = 'OptimizationReporter/adjoint'
    direction = from_multiapp
  []

  [toHomogenousForward]
    type = OptimizationParameterTransfer
    multi_app = homogenousForward
    value_names = 'fy_right'
    parameters = 'BCs/right_fy/value'
    to_control = parameterReceiver
  []
  [toHomogenousForward_measument]
    type = MultiAppReporterTransfer
    multi_app = homogenousForward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [fromHomogenousForward]
    type = MultiAppReporterTransfer
    multi_app = homogenousForward
    from_reporters = 'data_pt/disp_y'
    to_reporters = 'OptimizationReporter/simulation_values'
    direction = from_multiapp
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
  []
[]

[Outputs]
  csv = true
[]
