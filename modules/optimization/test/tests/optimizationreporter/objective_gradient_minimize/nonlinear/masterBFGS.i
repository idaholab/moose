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
  [forwardT]
  []
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'heat_source'
  num_values = '1'
  initial_condition = '000'
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
## THE FOLLOWING IS FOR REDUCED NEWTON METHOD WITH MATRIX-FREE HESSIAN
#  tao_solver = taonls
#  petsc_options_iname = '-tao_gttol -tao_max_it -tao_nls_pc_type -tao_nls_ksp_type'
#  petsc_options_value = ' 1e-5       5           none             cg'
#  verbose = true
## THE FOLLOWING IS FOR LIMITED MEMORY BFGS
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-12'
  verbose = true
## THE FOLLOWING IS FOR LIMITED MEMORY BFGS
#  tao_solver = taolmvm
#  petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'  # GRADIENT CHECK
#  petsc_options_value=' 3           true         true               false            0.0001        0.0001'
#  petsc_options = '-tao_test_gradient_view'
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
    clone_master_mesh = true
    reset_app = true
  []
  [homogeneous_forward]
    type = OptimizeFullSolveMultiApp
    input_files = homogeneous_forward.i
    execute_on = HOMOGENEOUS_FORWARD
    clone_master_mesh = true
    reset_app = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = ADJOINT
    clone_master_mesh = true
    reset_app = true
  []
[]

[Transfers]
## RUN FORWARD SIMULATION WITH CURRENT PARAMETERS AS FORCE,
## AND EXTRACT SIMULATED VALUES AT MEASUREMENT POINTS
## AS WELL AS TOTAL FIELD VARIABLE FOR NONLINEAR PURPOSES
  [ParametersToForward]
    type = OptimizationParameterTransfer
    to_multi_app = forward
    value_names = 'heat_source'
    parameters = 'Postprocessors/heat_source_pp/value'
    to_control = parameterReceiver #'Functions/volumetric_heat_func/vals'
  []
  [MeasurementLocationsToForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord'
    to_reporters = 'measurement_locations/measurement_xcoord
                    measurement_locations/measurement_ycoord
                    measurement_locations/measurement_zcoord'
  []
  [SimulatedDataFromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data_pt/forwardT'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  [CurrentStateFromForwardNonlinear]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    source_variable = 'forwardT'
    variable = 'forwardT'
  []
### RUN THE HOMOGENEOUS_FORWARD WITH CURRENT NONLINEAR STATE, PARAMTER_STEP,
### AND EXTRACT SIMULATED DATA AT MEASURMENT POINTS
  [CurrentStateToHomogeneousForwardNonlinear]
    type = MultiAppCopyTransfer
    to_multi_app = homogeneous_forward
    source_variable = 'forwardT'
    variable = 'forwardT'
  []
  [ParametersToHomogeneousForward]
    type = OptimizationParameterTransfer
    to_multi_app = homogeneous_forward
    value_names = 'heat_source'
    parameters = 'Postprocessors/heat_source_pp/value'
    to_control = parameterReceiver #'Functions/volumetric_heat_func/vals'
  []
  [MeasurementLocationsToHomogeneousForward]
    type = MultiAppReporterTransfer
    to_multi_app = homogeneous_forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord'
    to_reporters = 'measurement_locations/measurement_xcoord
                    measurement_locations/measurement_ycoord
                    measurement_locations/measurement_zcoord'
  []
  [SimulatedDataFromHomogeneousForward]
    type = MultiAppReporterTransfer
    from_multi_app = homogeneous_forward
    from_reporters = 'data_pt/T'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
### RUN THE ADJOINT WITH CURRENT NONLINEAR STATE, WITH MISFIT AS EXCITATION,
### AND EXTRACT GRADIENT
  [CurrentStateToAdjointNonlinear]
    type = MultiAppCopyTransfer
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
                      OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/misfit_values'
  []
  [GradientFromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradient_vpp/gradient_vpp'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Reporters]
   [optInfo]
     type = OptimizationInfo
   []
[]

[Outputs]
  csv=true
[]
