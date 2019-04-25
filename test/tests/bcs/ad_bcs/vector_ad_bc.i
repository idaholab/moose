[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    family = LAGRANGE_VEC
  [../]
[]

[Kernels]
  [./diff]
    type = ADVectorDiffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = ADVectorFunctionDirichletBC
    variable = u
    boundary = left
    function_x = '1'
    function_y = '1'
  [../]
  [./right]
    type = ADVectorRobinBC
    variable = u
    boundary = right
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
