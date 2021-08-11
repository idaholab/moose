[StochasticTools]
[]

[FormFunction]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '5'

  misfit_name = misfit
  adjoint_data_name = adjoint
[]

[Executioner]
  type = Optimize
  tao_solver = taonm
  petsc_options_iname='-tao_max_it -tao_gatol -tao_ls_type'
  petsc_options_value='1000 1e-1 unit'
  #tao_solver = taolmvm #TAOOWLQN #TAOBMRM #taolmvm #taocg
  #petsc_options_iname = '-tao_gatol'# -tao_cg_delta_max'
  #petsc_options_value = '1e-2'

   # tao_solver = taocg
   # petsc_options = '-tao_test_gradient_view'
   # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
   # petsc_options_value='1 true true false 0.0001 0.0001'
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
    from_reporters = 'forward_meas/temperature_difference forward_meas/temperature'
    to_reporters = 'FormFunction/misfit measured/values'
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
    from_reporters = 'adjoint_meas/temperature'
    to_reporters = 'FormFunction/adjoint'
  []
[]

[Reporters]
  [measured]
    type = ConstantReporter
    real_vector_names = values
    real_vector_values = '0'
  []
  [optInfo]
    type = OptimizationInfo
  []
[]

[Outputs]
  csv=true
[]
