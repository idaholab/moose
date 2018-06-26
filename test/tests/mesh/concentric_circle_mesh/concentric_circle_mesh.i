[Mesh]
  type = ConcentricCircleMesh
  unit_cell_length = 0.496
  radius_fuel = 0.3225
  outer_radius_clad = 0.374
  inner_radius_clad = 0.3515
  num_sectors = 4
  nr = 3
  num_intervals_unit_cell = 3

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
