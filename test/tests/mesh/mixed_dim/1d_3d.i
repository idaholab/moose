[Mesh]
  file = 1d_3d.e
  # 1d_3d.e contains HEX8 and BEAM2 elements - no additional input file
  # changes are necessary to handle mixed-dim meshes.
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
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
    boundary = 4
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 100
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 101
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = 1d_3d_out
  exodus = true
[]
