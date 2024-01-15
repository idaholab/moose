# DO NOT CHANGE THIS TEST
# this test is documented as an example in forceInv_pointLoads.md
# if this test is changed, the figures will need to be updated.
measurement_points = '0.5 0.28 0
   0.5 0.6 0
   0.5 0.8 0
   0.5 1.1 0'
measurement_values = '293 304 315 320'
[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'parameter_results'
  num_values = '3'
[]

[Reporters]
  [main]
    type = OptimizationData
    measurement_points = ${measurement_points}
    measurement_values = ${measurement_values}
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taonls
  petsc_options_iname = '-tao_gttol -tao_max_it -tao_nls_pc_type -tao_nls_ksp_type'
  petsc_options_value = '1e-5 10 none cg'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
  []
  [homogeneousForward]
    type = FullSolveMultiApp
    input_files = forward_homogeneous.i
    execute_on = "HOMOGENEOUS_FORWARD"
  []
[]

[Transfers]
  # FORWARD transfers
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/measurement_values
                      OptimizationReporter/parameter_results'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    point_source/value'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    # Note: We are transferring the misfit values into main misfit
    from_reporters = 'measure_data/objective_value measure_data/misfit_values'
    to_reporters = 'OptimizationReporter/objective_value main/misfit_values'
  []

  # ADJOINT transfers
  #NOTE:  the adjoint variable we are transferring is actually the gradient
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/misfit_values'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradient/adjoint'
    to_reporters = 'OptimizationReporter/grad_parameter_results'
  []

  # HESSIAN transfers.  Same as forward.
  [toHomoForward]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = to_multiapp
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/measurement_values
                      OptimizationReporter/parameter_results'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    point_source/value'
  []
  [fromHomoForward]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = from_multiapp
    # Note: We are transferring the simulation values into misfit
    # this has to be done when using general opt and homogenous forward.
    from_reporters = 'measure_data/simulation_values'
    to_reporters = 'main/misfit_values'
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
