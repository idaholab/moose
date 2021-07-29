[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '3'
  initial_condition = '1 1 1'
<<<<<<< HEAD:test/tests/optimizationreporter/objective_gradient_minimize/function_load_quadratic/master.i

=======
  # lower_bounds = '-5000 0     -1000'
  # upper_bounds = '0     10000  0'
  misfit_name = 'misfit'
>>>>>>> 5d604ba (changes for milestone report):test/tests/formfunction/objective_gradient_minimize/function_load_quadratic/master.i
  adjoint_data_name = 'adjoint'
[]

[Executioner]
  type = Optimize
  tao_solver = taocg #taolmvm
  petsc_options_iname = '-tao_max_it -tao_fmin -tao_ls_type' # tao_gatol'
  petsc_options_value = '50 4 unit' #1e-4
   # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
   # petsc_options_value='1 true true false 0.0001 0.0001'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
    reset_app = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = ADJOINT
    reset_app = true
  []
[]

[Controls]
  [toforward]
    type = OptimizationMultiAppCommandLineControl
    multi_app = forward
    value_names = 'parameter_results'
    parameters = 'Functions/volumetric_heat_func/vals'
  []
  [toadjoint]
    type = OptimizationMultiAppCommandLineControl
    multi_app = adjoint
    value_names = 'parameter_results'
    parameters = 'function_vals'
  []
[]

[Transfers]
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    from_reporters = 'data_pt/temperature_difference data_pt/temperature'
    to_reporters = 'OptimizationReporter/misfit receiver/measured'
    direction = from_multiapp
  []
  [toadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    from_reporters = 'OptimizationReporter/misfit'
    to_reporters = 'point_source/value'
    direction = to_multiapp
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    from_reporters = 'adjoint_pt/adjoint_pt'
    to_reporters = 'OptimizationReporter/adjoint'
    direction = from_multiapp
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0 0 0 0'
  []
  [optInfo]
    type = OptimizationInfo
  []
[]

[Outputs]
  # console = true
  csv=true
[]
