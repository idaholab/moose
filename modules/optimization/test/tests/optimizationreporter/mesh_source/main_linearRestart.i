[Optimization]
[]

[OptimizationReporter]
  type = ParameterMeshOptimization
  objective_name = objective_value
  parameter_names = 'source'
  parameter_meshes = 'parameter_mesh_restart_out.e'
  exodus_timesteps_for_parameter_mesh_variable = 2
  initial_condition_mesh_variable = restart_source
  lower_bounds = -1
  upper_bounds = 5
  outputs = none
[]

[Reporters]
  [main]
    type = OptimizationData
    # Random points
    measurement_points = '0.78193073 0.39115321 0
  0.72531893 0.14319403 0
  0.14052488 0.86976625 0
  0.401893   0.54241797 0
  0.02645427 0.43320192 0
  0.28856889 0.0035165  0
  0.51433644 0.94485949 0
  0.29252255 0.7962032  0
  0.04925654 0.58018889 0
  0.04717357 0.9556314  0'
    # sin(x*pi/2)*sin(y*pi/2)
    measurement_values = '0.54299466 0.20259611 0.21438235 0.44418597 0.02613676
  0.00241892 0.72014019 0.42096307 0.06108895 0.07385256'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-4'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = ADJOINT
  []
[]

[Transfers]
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/measurement_values
                      OptimizationReporter/source'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    src_rep/vals'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/misfit_values
                      OptimizationReporter/source'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    src_rep/vals'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/misfit_values measure_data/objective_value'
    to_reporters = 'main/misfit_values OptimizationReporter/objective_value'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradient_vpp/inner_product'
    to_reporters = 'OptimizationReporter/grad_source'
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
[]
