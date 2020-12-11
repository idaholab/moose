[StochasticTools]
[]

[FormFunction]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '8'

  misfit_name = misfit
  adjoint_data_name = adjoint
[]

[Executioner]
  type = Optimize
  # tao_solver = taonm
  # petsc_options_iname='-tao_gatol'
  # petsc_options_value='1e-2'
  tao_solver = taolmvm #TAOOWLQN #TAOBMRM #taolmvm #taocg
  petsc_options_iname = '-tao_gatol'# -tao_cg_delta_max'
  petsc_options_value = '1e-2'
  # tao_solver = taontr
  # petsc_options_iname='-tao_fd_hessian -tao_fd_delta -tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type -tao_gatol'
  # petsc_options_value='true 0.000001 0 1e16 constant 1e-2'
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
    from_reporters = 'dr/temperature_difference dr/temperature'
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
    from_reporters = 'ar/temperature'
    to_reporters = 'FormFunction/adjoint'
  []
[]

[Reporters]
  [measured]
    type = ConstantReporter
    real_vector_names = values
    real_vector_values = '0'
  []
[]

[Outputs]
  csv=true
[]
