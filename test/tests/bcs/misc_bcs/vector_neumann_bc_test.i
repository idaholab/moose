[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right top'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0.0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 2.0
  [../]

  [./top]
    type = VectorNeumannBC
    variable = u
    vector_value = '1 1 0'
    boundary = 2
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
