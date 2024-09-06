[Optimization]
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'youngs_modulus'
  num_values = '3'
  initial_condition = '5.0 5.0 5.0'
  lower_bounds = '0.1'
  upper_bounds = '10.0'
[]

[Reporters]
  [main]
    type = OptimizationData
    measurement_points = '-1.0 -1.0 0.0
                          -1.0  0.0 0.0
                          -1.0  1.0 0.0
                           0.0 -1.0 0.0
                           0.0  0.0 0.0
                           0.0  1.0 0.0
                           1.0 -1.0 0.0
                           1.0  0.0 0.0
                           1.0  1.0 0.0'
    measurement_values = '3.276017e+00
                          4.763281e+00
                          6.380137e+00
                          3.171603e+00
                          4.660766e+00
                          6.289842e+00
                          3.127077e+00
                          4.608134e+00
                          6.228638e+00'
  []
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnls #taobncg #taoblmvm
  petsc_options_iname = '-tao_gatol -tao_ls_type -tao_max_it'
  petsc_options_value = '1e-10 unit 1000'

  # THESE OPTIONS ARE FOR TESTING THE ADJOINT GRADIENT
  # petsc_options_iname = '-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value = '1 true true false 1e-8 0.1'
  # petsc_options = '-tao_test_gradient_view'
  # verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = grad.i
    execute_on = ADJOINT
  []
[]

[Transfers]
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/measurement_values
                      OptimizationReporter/youngs_modulus'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    parametrization/youngs_modulus'
  []
  [get_misfit]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/misfit_values measure_data/objective_value'
    to_reporters = 'main/misfit_values OptimizationReporter/objective_value'
  []
  [set_state_for_adjoint]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    to_multi_app = adjoint
    source_variable = 'disp_x disp_y'
    variable = 'disp_x disp_y'
  []
  [setup_adjoint_run]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'main/measurement_xcoord
                      main/measurement_ycoord
                      main/measurement_zcoord
                      main/measurement_time
                      main/misfit_values
                      OptimizationReporter/youngs_modulus'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    parametrization/youngs_modulus'
  []
  [get_grad_youngs_modulus]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'grad_youngs_modulus/inner_product'
    to_reporters = 'OptimizationReporter/grad_youngs_modulus'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
    items = 'current_iterate function_value gnorm'
  []
[]

[Outputs]
  console = false
  csv = true
[]
