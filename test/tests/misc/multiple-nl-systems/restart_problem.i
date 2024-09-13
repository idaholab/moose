[Mesh]
  file = 'gold/restart_problem_out.e'
[]

[Problem]
  nl_sys_names = 'u v'
  allow_initial_conditions_with_restart = true
  solve = false
[]

[Variables]
  [u]
    solver_sys = 'u'
    initial_from_file_var = u
    initial_from_file_timestep = LATEST
  []
  [v]
    solver_sys = 'v'
    initial_from_file_var = v
    initial_from_file_timestep = LATEST
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [force]
    type = CoupledForce
    variable = v
    v = u
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1.01
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1.01
  []
[]

[Executioner]
  type = SteadySolve2
  solve_type = 'NEWTON'
  petsc_options = '-snes_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  first_nl_sys_to_solve = 'u'
  second_nl_sys_to_solve = 'v'
[]
[Outputs]
  exodus = true
  execute_on = FINAL
[]
