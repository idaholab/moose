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

[GPUKernels]
  [./diff]
    type = GPUDiffusion
    variable = u
  [../]
[]

[GPUBCs]
  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = GPUNeumannBC
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
