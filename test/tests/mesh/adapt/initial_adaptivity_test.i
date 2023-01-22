[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = x*x+y*y
  [../]

  [./ffn]
    type = ParsedFunction
    expression = -4
  [../]
[]

[Variables]
  [./u]
    order = SECOND
    family = LAGRANGE
    [./InitialCondition]
      type = BoundingBoxIC
      x1 = -2
      y1 = -2
      x2 =  0
      y2 =  2
      inside = 1
      outside = 0
    [../]
  [../]
[]

[Kernels]
  [./udiff]
    type = Diffusion
    variable = u
  [../]

  [./forcing_fn]
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
    function = exact_fn
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  [./Adaptivity]
    initial_adaptivity = 5
    refine_fraction = 0.2
    coarsen_fraction = 0.3
    max_h_level = 4
  [../]
[]

[Outputs]
  exodus = true
[]
