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

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'heat_source'
  num_values = '1'
  initial_condition = '0'
  lower_bounds = '0.1'
  upper_bounds = '10000'
  measurement_points = '0.2 0.2 0
                        0.8 0.6 0
                        0.2 1.4 0
                        0.8 1.8 0'
  measurement_values = '1.98404 1.91076 1.56488 1.23863'
[]

[Executioner]
  type = Optimize
  tao_solver = taonls
  petsc_options_iname = '-tao_gttol -tao_max_it -tao_nls_pc_type -tao_nls_ksp_type'
  petsc_options_value = ' 1e-5       5           none             cg'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
    clone_parent_mesh = true
  []
  [homogeneous_forward]
    type = FullSolveMultiApp
    input_files = homogeneous_forward.i
    execute_on = HOMOGENEOUS_FORWARD
    clone_parent_mesh = true
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = ADJOINT
    clone_parent_mesh = true
  []
[]

[Transfers]
  ## RUN FORWARD SIMULATION WITH CURRENT PARAMETERS AS FORCE,
  ## AND EXTRACT SIMULATED VALUES AT MEASUREMENT POINTS
  ## AS WELL AS TOTAL FIELD VARIABLE FOR NONLINEAR PURPOSES
  [MeasurementLocationsToForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/heat_source'
    to_reporters = 'measurement_locations/measurement_xcoord
                    measurement_locations/measurement_ycoord
                    measurement_locations/measurement_zcoord
                    measurement_locations/measurement_time
                    measurement_locations/measurement_values
                    params/heat_source'
  []
  [SimulatedDataFromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measurement_locations/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  ### RUN THE HOMOGENEOUS_FORWARD WITH CURRENT NONLINEAR STATE, PARAMETER_STEP,
  ### AND EXTRACT SIMULATED DATA AT MEASURMENT POINTS
  [CurrentStateFromForwardNonlinearToHomogeneousForwardNonlinear]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    to_multi_app = homogeneous_forward
    source_variable = 'forwardT'
    variable = 'forwardT'
  []
  [MeasurementLocationsToHomogeneousForward]
    type = MultiAppReporterTransfer
    to_multi_app = homogeneous_forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/heat_source'
    to_reporters = 'measurement_locations/measurement_xcoord
                    measurement_locations/measurement_ycoord
                    measurement_locations/measurement_zcoord
                    measurement_locations/measurement_time
                    measurement_locations/measurement_values
                    params/heat_source'
  []
  [SimulatedDataFromHomogeneousForward]
    type = MultiAppReporterTransfer
    from_multi_app = homogeneous_forward
    from_reporters = 'measurement_locations/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  ### RUN THE ADJOINT WITH CURRENT NONLINEAR STATE, WITH MISFIT AS EXCITATION,
  ### AND EXTRACT GRADIENT
  [CurrentStateToAdjointNonlinear]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    to_multi_app = adjoint
    source_variable = 'forwardT'
    variable = 'forwardT'
  []
  [MisfitToAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/misfit_values
                      OptimizationReporter/heat_source'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    params/heat_source'
  []
  [GradientFromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradient_vpp/inner_product'
    to_reporters = 'OptimizationReporter/grad_heat_source'
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
