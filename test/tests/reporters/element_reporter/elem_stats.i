[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 2
    ymax = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Reporters]
  [elem_stats]
    type = ElementVariableStatistics
    coupled_var = u
    base_name = diffusion
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  [stats]
    type = JSON
    execute_system_information_on = none
  []
[]
