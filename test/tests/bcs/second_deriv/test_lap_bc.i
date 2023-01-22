[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
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
  [./ffn]
    type = BodyForce
    variable = u
    function = force_fn
  [../]
[]

[Functions]
  [./left_bc_func]
    type = ParsedFunction
    expression = '1+y*y'
  [../]
  [./top_bc_func]
    type = ParsedFunction
    expression = '1+x*x'
  [../]
  [./bottom_bc_func]
    type = ParsedFunction
    expression = '1+x*x'
  [../]
  [./force_fn]
    type = ParsedFunction
    expression = -4
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = left_bc_func
  [../]
  [./bottom]
    type = FunctionDirichletBC
    variable = u
    boundary = bottom
    function = bottom_bc_func
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = u
    boundary = top
    function = top_bc_func
  [../]
  [./right_test]
    type = TestLapBC
    variable = u
    boundary = right
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = out
  exodus = true
[]
