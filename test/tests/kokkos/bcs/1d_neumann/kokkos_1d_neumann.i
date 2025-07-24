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

[KokkosKernels]
  [./diff]
    type = KokkosDiffusion
    variable = u
  [../]
[]

[KokkosBCs]
  [./left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = KokkosNeumannBC
    variable = u
    boundary = right
    value = 2
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
