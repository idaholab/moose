[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[ADKernels]
  [./diff]
    type = ADDiffusion
    variable = u
  [../]
[]

[ADBCs]
  [./left]
    type = ADFunctionDirichletBC
    variable = u
    boundary = left
    function = '1'
  [../]
  [./right]
    type = ADRobinBC
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
