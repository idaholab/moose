[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
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
    preset = false
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
