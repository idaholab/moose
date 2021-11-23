[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '3'
  measurement_points = '0.3 0.3 0
            0.4 1.0 0
            0.8 0.5 0
            0.8 0.6 0'
  measurement_values = '100 204 320 216'
[]

[Executioner]
  type = Optimize
  tao_solver = taobncg
  petsc_options_iname = '-tao_gatol -tao_max_it'
  petsc_options_value = '1e-1 50'
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
[]

[Transfers]
  [toforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  [toadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_points OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_points misfit/misfit_values'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/adjoint'
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
    #items = 'current_iterate'
    #execute_on=timestep_end
  []
[]

[Outputs]
  console = true
  csv=true
  json=true
[]
