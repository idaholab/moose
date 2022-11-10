# DO NOT CHANGE THIS TEST
# this test is documented as an example in forceInv_pointLoads.md
# if this test is changed, the figures will need to be updated.
[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'parameter_results'
  num_values = '3'
  measurement_points = '0.5 0.28 0
                        0.5 0.6 0
                        0.5 0.8 0
                        0.5 1.1 0'
  measurement_values = '293 304 315 320'
[]

[Executioner]
  type = Optimize
  tao_solver=taonls
  petsc_options_iname='-tao_gttol -tao_max_it -tao_nls_pc_type -tao_nls_ksp_type'
  petsc_options_value='1e-5 10 none cg'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
  []
  [homogeneousForward]
    type = OptimizeFullSolveMultiApp
    input_files = forward_homogeneous.i
    execute_on = "HOMOGENEOUS_FORWARD"
  []
[]

[Transfers]
  # FORWARD transfers
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values'
  []

  # ADJOINT transfers
  #NOTE:  the adjoint variable we are transferring is actually the gradient
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradient/adjoint'
    to_reporters = 'OptimizationReporter/adjoint'
  []

  # HESSIAN transfers.  Same as forward.
  [toHomoForward_measument]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [toHomoForward]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromHomoForward]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
  []
[]

[Outputs]
  console = true
  csv = true
  json = false
[]
