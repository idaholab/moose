[Optimization]
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
  type = OptimizationReporter
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
  # the forward problem has homogeneous boundary conditions so it can be reused here.
  [homogeneousForward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = "HOMOGENEOUS_FORWARD"
    clone_parent_mesh = true
  []
[]

[Transfers]
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/fy_right'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/right_fy_value'
  []
  [fromforward]
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
                      OptimizationReporter/fy_right'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    params/right_fy_value'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_pt/inner_product'
    to_reporters = 'OptimizationReporter/grad_fy_right'
  []

  [toHomogeneousForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = homogeneousForward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/fy_right'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/right_fy_value'
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
  []
[]

[Outputs]
  csv = true
[]
