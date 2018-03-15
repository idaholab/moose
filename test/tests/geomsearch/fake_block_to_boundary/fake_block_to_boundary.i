[Mesh]
  type = FileMesh
  file = fake_geom_search.e
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./distance_to_left_nodes]
  [../]
  [./penetration_to_left]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./nodal_distance_aux]
    type = NearestNodeDistanceAux
    variable = distance_to_left_nodes
    boundary = 100
    paired_boundary = left
  [../]
  [./penetration_aux]
    type = PenetrationAux
    variable = penetration_to_left
    boundary = 100
    paired_boundary = left
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
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

