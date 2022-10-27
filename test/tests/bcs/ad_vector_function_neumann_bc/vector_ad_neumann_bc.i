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
  [./top_bottom]
    type = ADVectorFunctionDirichletBC
    variable = u
    boundary = 'top bottom'
  [../]
  [./left]
    type = ADVectorFunctionNeumannBC
    variable = u
    boundary = left
    function_x = '1'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
