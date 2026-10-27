!include flow.i

[Executioner]
  type = PIMPLE
  rhie_chow_user_object = ins_rhie_chow_interpolator
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system

  num_iterations = 1
  continue_on_max_its = true

  momentum_petsc_options_iname = '-pc_type'
  momentum_petsc_options_value = 'jacobi'
  pressure_petsc_options_iname = '-pc_type'
  pressure_petsc_options_value = 'jacobi'

  dt = 0.01
  num_steps = 1
[]

[MultiApps]
  inactive = eigen
  [eigen]
    type = FullSolveMultiApp
    input_files = eigen.i
    execute_on = timestep_end
  []
[]

