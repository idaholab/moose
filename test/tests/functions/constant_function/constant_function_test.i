[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[Functions]
  [./bc_fn]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]

  [./icfn]
    type = ConstantFunction
    value = 1
  [../]

  [./ffn]
    type = ConstantFunction
    value = -4
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = icfn
    [../]
  [../]
[]

[Kernels]
  # Coupling of nonlinear to Aux
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = bc_fn
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

[]

[Outputs]
  exodus = true
[]
