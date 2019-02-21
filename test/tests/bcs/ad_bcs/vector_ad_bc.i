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
    type = ADLagrangeVecFunctionDirichletBC
    variable = u
    boundary = left
    x_exact_soln = '1'
    y_exact_soln = '1'
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
