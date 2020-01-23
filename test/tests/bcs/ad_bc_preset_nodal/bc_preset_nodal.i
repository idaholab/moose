[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = ADDiffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  # We will use preset BCs
  [./left]
    type = ADDirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = ADDirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = bc_preset_out
  exodus = true
[]
