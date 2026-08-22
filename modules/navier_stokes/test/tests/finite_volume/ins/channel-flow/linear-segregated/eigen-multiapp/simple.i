!include flow.i

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = ins_rhie_chow_interpolator
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system

  num_iterations = 1
  continue_on_max_its = true

  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'jacobi'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'jacobi'
[]

[MultiApps]
  inactive = eigen
  [eigen]
    type = FullSolveMultiApp
    input_files = eigen.i
    execute_on = timestep_end
  []
[]

