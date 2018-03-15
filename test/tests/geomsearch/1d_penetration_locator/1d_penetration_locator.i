[Mesh]
  file = 1d_contact.e
  dim = 2
  construct_side_list_from_node_list = true
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./gap_distance]
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
    boundary = left_left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right_right
    value = 1
  [../]
[]

[AuxKernels]
  [./distance]
    type = PenetrationAux
    variable = gap_distance
    boundary = left_right
    paired_boundary = right_left
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
