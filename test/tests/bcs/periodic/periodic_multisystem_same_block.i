[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 1
  ny = 1
[]

[Variables]
  [u]
    solver_sys = u
  []
  [v]
    solver_sys = v
  []
[]

[BCs/Periodic/periodic]
  variable = 'u v'
  auto_direction = x
[]

[Executioner]
  type = Steady
  solve = false
[]

[Problem]
  nl_sys_names = 'u v'
[]
