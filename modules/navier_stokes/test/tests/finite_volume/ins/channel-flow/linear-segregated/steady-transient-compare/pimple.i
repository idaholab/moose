[Times]
  [time_steps]
    type = InputTimes
    times = '1 1E2 1E4 1E5 1E6 1E7 2E7 4E7 6E7 8E7 1E8'
  []
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-11
  pressure_l_abs_tol = 1e-11
  energy_l_abs_tol = 1e-11
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.9
  energy_equation_relaxation = 1.0
  pressure_variable_relaxation = 0.3
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  end_time = 1e8

  [TimeStepper]
    type = TimeSequenceFromTimes
    times = time_steps
  []
[]
