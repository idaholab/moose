[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  construct_side_list_from_node_list = true
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = ADNeumannBC
    variable = u
    boundary = right
    value = 2
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
