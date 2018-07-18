[Mesh]
  type = ConcentricCircleMesh
  num_sectors = 6
  radii = '0.16125 0.1645 0.187'
  rings = '4 1 3 3'
  inner_mesh_fraction = 0.6
  volume_preserving_function = on
  #portion = left_half
  pitch = 0.496
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
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  #solve_type = 'PJFNK'
  #petsc_options_iname = '-pc_type -pc_hypre_type'
  #petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
