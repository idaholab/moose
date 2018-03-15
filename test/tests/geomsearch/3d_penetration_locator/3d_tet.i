[Mesh]
  file = 3d_thermal_contact_tet.e
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
    boundary = leftleft
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = rightright
    value = 1
  [../]
[]

[AuxKernels]
  [./distance]
    type = PenetrationAux
    variable = gap_distance
    boundary = leftright
    paired_boundary = rightleft
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
