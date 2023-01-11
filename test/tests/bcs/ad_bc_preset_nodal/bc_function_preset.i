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

[Functions]
  [./left]
    type = ParsedFunction
    expression = 'y'
  [../]

  [./right]
    type = ParsedFunction
    expression = '1+y'
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

   [./left]
    type = ADFunctionDirichletBC
    variable = u
    boundary = 3
    function = left
  [../]

  [./right]
    type = ADFunctionDirichletBC
    variable = u
    boundary = 1
    function = right
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = bc_func_out
  exodus = true
[]
