!include base.i

[Executioner]
  type = Steady
  system_names = 'v_sys u_sys w_sys'
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  l_tol = 1e-10
[]
