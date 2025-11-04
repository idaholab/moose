[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = restart_from.e
    use_for_exodus_restart = true
  []
[]

[Problem]
  linear_sys_names = "u_sys"
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    initial_from_file_var = u
    solver_sys = "u_sys"
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
