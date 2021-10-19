[StochasticTools]
[]

[FormFunction]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '3'
  misfit_name = 'misfit'
  adjoint_data_name = 'adjoint'
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
    from_reporters = 'FormFunction/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = from_multiapp
    from_reporters = 'data_pt/temperature_difference data_pt/temperature'
    to_reporters = 'FormFunction/misfit receiver/measured'
  []

  [toadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = to_multiapp
    from_reporters = 'FormFunction/misfit'
    to_reporters = 'point_source/value'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'FormFunction/adjoint'
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
[]
