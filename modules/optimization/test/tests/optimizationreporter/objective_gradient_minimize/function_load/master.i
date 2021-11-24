[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '4'
  initial_condition = '100 1 -10 -10'
  points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measured_values = '209 218 164 121'
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-4'
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
  [fromForward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = from_multiapp
    #the second vector in the reporterTransfer just writes to teh constantReporter below for the csvDiff in the test file
    from_reporters = 'data_pt/temperature data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_points OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_points misfit/misfit_values'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'adjoint_pt/adjoint_pt'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0 0 0 0'
  []
[]

[Outputs]
  console = true
  csv=true
[]
