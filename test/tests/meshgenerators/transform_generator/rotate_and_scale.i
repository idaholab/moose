[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = cylinder.e
  []

  [./rotate]
    type = TransformGenerator
    input = fmg
    transform = ROTATE
    vector_value = '0 90 0'
  []

  [./scale]
    type = TransformGenerator
    input = rotate
    transform = SCALE
    vector_value ='1e2 1e2 1e2'
  []
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
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
