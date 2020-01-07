[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD4
  nx = 8
  ny = 8
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
  skip_exception_check = true
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
