# This tests that a linear and constant function can be scaled in
# two seperate functionNeumannBCs both applied to the same sideset using
# two parsed functions.  The scale of the linear and constant functions
# are being parameterized.

[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
  bias_x = 1.1
  bias_y = 1.1
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'p1 p2'
  num_values = '1 1'
  measurement_file = 'measurementData.csv'
  file_xcoord = 'coordx'
  #file_ycoord ='y'
  file_zcoord ='z'
  file_value = 'measured_value'
  # contents of measurementData.csv
  # measurement_points = '0.2 0.2 0
  #                       0.8 0.6 0
  #                       0.2 1.4 0
  #                       0.8 1.8 0'
  # measurement_values = '207 204 185 125'
[]

[Executioner]
  type = Optimize
  # tao_solver = taobncg
  # petsc_options_iname = '-tao_gatol'
  # petsc_options_value = '1e-4'

  tao_solver = taonls
  petsc_options_iname = '-tao_gttol -tao_nls_pc_type -tao_nls_ksp_type'
  petsc_options_value = '1e-5 none cg'

  # tao_solver = taonm
  # petsc_options_iname = '-tao_max_it -tao_gatol'
  # petsc_options_value = '10000 1e-4'

  # tao_solver = taoblmvm
  # petsc_options_iname = '-tao_max_it -tao_gatol'
  # petsc_options_value = '10000 1e-4'

  # petsc_options_iname = '-tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value = 'true 0.0001 1e-4'

  # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_test_hessian -tao_fd_delta -tao_gatol -tao_nls_pc_type -tao_nls_ksp_type'
  # petsc_options_value=' 1           true         true               true              0.0001        0.0001     none             cg'
  verbose = true
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
  [homogeneousForward]
    type = OptimizeFullSolveMultiApp
    input_files = homogeneous_forward.i
    execute_on = "HOMOGENEOUS_FORWARD"
    clone_master_mesh = true
  []
[]

[Transfers]
  #these are usually the same for all input files.
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
    [toForward_measument]
      type = MultiAppReporterTransfer
      to_multi_app = forward
      from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
      to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
    []
  #these are different,
  # - to forward depends on teh parameter being changed
  # - from adjoint depends on the gradient being computed from the adjoint
  #NOTE:  the adjoint variable we are transferring is actually the gradient

  [toforward]
    type = OptimizationParameterTransfer
    to_multi_app = forward
    value_names = 'p1 p2'
    parameters = 'Postprocessors/p1/value Postprocessors/p2/value'
    to_control = parameterReceiver
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_bc/adjoint_bc' # what is the naming convention for this
    to_reporters = 'OptimizationReporter/adjoint'
  []

  # HESSIAN transfers.  Same as forward.
  [fromHomogeneousForward]
    type = MultiAppReporterTransfer
    from_multi_app = homogeneousForward
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  [toHomogeneousForward]
    type = OptimizationParameterTransfer
    to_multi_app = homogeneousForward
    value_names = 'p1 p2'
    parameters = 'Postprocessors/p1/value Postprocessors/p2/value'
    to_control = parameterReceiver
  []
  [toHomogeneousForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = homogeneousForward
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
[]

[Reporters]
   [optInfo]
     type = OptimizationInfo
     items = 'current_iterate function_value gnorm'
   []
[]

[Outputs]
  csv=true
  console = false
[]
