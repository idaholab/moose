# This tests that a linear and constant function can be scaled in
# two separate functionNeumannBCs both applied to the same sideset using
# two parsed functions.  The scale of the linear and constant functions
# are being parameterized.

[Optimization]
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
  type = OptimizationReporter
  parameter_names = 'p'
  num_values = '2'
  measurement_file = 'measurementData.csv'
  file_xcoord = 'coordx'
  file_ycoord ='y'
  file_zcoord = 'z'
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

  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_parent_mesh = true
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_parent_mesh = true
  []
  [homogeneousForward]
    type = FullSolveMultiApp
    input_files = homogeneous_forward.i
    execute_on = "HOMOGENEOUS_FORWARD"
    clone_parent_mesh = true
  []
[]

[Transfers]
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/p'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/vals'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/misfit_values
                      OptimizationReporter/p'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    params/vals'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_bc/inner_product'
    to_reporters = 'OptimizationReporter/adjoint'
  []

  # HESSIAN transfers.  Same as forward.
  [toHomogeneousForward]
    type = MultiAppReporterTransfer
    to_multi_app = homogeneousForward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/p'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/vals'
  []
  [fromHomogeneousForward]
    type = MultiAppReporterTransfer
    from_multi_app = homogeneousForward
    from_reporters = 'measure_data/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
    items = 'current_iterate function_value gnorm'
  []
[]

[Outputs]
  csv = true
  console = false
[]
