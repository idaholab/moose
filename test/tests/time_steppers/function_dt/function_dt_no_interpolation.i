[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax =  1
  ymin = -1
  ymax =  1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t*t*(x*x+y*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = 2*t*(x*x+y*y)-4*t*t
  [../]

  [./dts]
    type = PiecewiseConstant
    x = '0  4  8 12  20'
    y = '0  1  2  4  8'
    direction = right
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[ICs]
  [./u_var]
    type = FunctionIC
    variable = u
    function = exact_fn
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient

  start_time = 0
  end_time = 20
  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Outputs]
  exodus = true
[]
