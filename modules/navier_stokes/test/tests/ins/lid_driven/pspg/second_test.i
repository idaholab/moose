[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1.0
  ymin = 0
  ymax = 1.0
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = UserForcingFunction
    function = ffn
    variable = u
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    function = soln
    variable = u
    boundary = 'left right top bottom'
  [../]
[]


[Functions]
  [./soln]
    type = ParsedFunction
    value = '5*x^3 + 4*y^3'
  [../]
  [./ffn]
    type = ParsedFunction
    value = '-30*x - 24*y'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
